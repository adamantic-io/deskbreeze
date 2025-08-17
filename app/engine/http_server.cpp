#include "http_server.hpp"
#include "common_defs.hpp"
#include "engine/incbin_common.h"

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <httplib.h>
#include <zip.h>


#include <string>
#include <memory>
#include <optional>
#include <vector>

namespace dbr {

using json = nlohmann::json;
using namespace std;
using httplib::Request;
using httplib::Response;

INCBIN_EXTERN(www_zip);


std::optional<std::vector<unsigned char>>
zip_extract_file_from_memory(const void* zipData, size_t zipSize, const char* pathInZip) {
    zip_error_t err; zip_error_init(&err);
    zip_source_t* src = zip_source_buffer_create(zipData, zipSize, 0, &err);
    if (!src) return std::nullopt;

    zip_t* za = zip_open_from_source(src, ZIP_RDONLY, &err);
    if (!za) { zip_source_free(src); return std::nullopt; }

    zip_int64_t idx = zip_name_locate(za, pathInZip, 0);
    if (idx < 0) { zip_close(za); return std::nullopt; }

    zip_stat_t st; if (zip_stat_index(za, idx, 0, &st) != 0) { zip_close(za); return std::nullopt; }

    const uint64_t MAX_SIZE = 64ull * 1024 * 1024; // safety cap
    if (st.size > MAX_SIZE) { zip_close(za); return std::nullopt; }

    zip_file_t* zf = zip_fopen_index(za, idx, 0);
    if (!zf) { zip_close(za); return std::nullopt; }

    std::vector<unsigned char> out(st.size);
    zip_int64_t rd = zip_fread(zf, out.data(), out.size());
    zip_fclose(zf);
    zip_close(za);

    if (rd != static_cast<zip_int64_t>(out.size())) return std::nullopt;
    return out;
}

template <typename T=std::string> optional<T> get_www_asset(const string& path) 
{
    auto data = zip_extract_file_from_memory(g_www_zip_data, g_www_zip_size, path.c_str());
    if (!data) return nullopt;
    return T(data->begin(), data->end());
}

string resolve_content_type(const string& path) {
    if (path.ends_with(".html")) return "text/html";
    if (path.ends_with(".js")) return "application/javascript";
    if (path.ends_with(".css")) return "text/css";
    if (path.ends_with(".png")) return "image/png";
    if (path.ends_with(".jpg") || path.ends_with(".jpeg")) return "image/jpeg";
    if (path.ends_with(".gif")) return "image/gif";
    if (path.ends_with(".svg")) return "image/svg+xml";
    if (path.ends_with(".json")) return "application/json";
    if (path.ends_with(".txt")) return "text/plain";
    // Add more content types as needed
    return "application/octet-stream"; // Default fallback
}

Server::~Server() { }

ErrorCode Server::own_configure(httplib::Server&) {
    spdlog::warn("Default configuration not overridden");
    return ErrorCode::Success;
}

ErrorCode Server::start() {
    if (is_running()) {
        return ErrorCode::AlreadyRunning;
    }

    if (!is_configured_) {
        auto res = configure();
        if (res != ErrorCode::Success) {
            return res;
        }
    }
    
    if (!srv_->bind_to_port("localhost", 3001)) {
        return ErrorCode::BindingFailed;
    }
    thread_ = std::make_unique<std::thread>([this]() {
        srv_->listen_after_bind();
    });
    thread_->detach();

    return ErrorCode::Success;
}

ErrorCode Server::stop() {
    if (!is_running()) {
        return ErrorCode::NotRunning;
    }
    srv_->stop();
    if (thread_ && thread_->joinable()) {
        thread_->join();
    }
    thread_.reset();
    return ErrorCode::Success;
}

ErrorCode Server::configure() {
    spdlog::debug("Starting server config...");
    if (is_configured_) {
        return ErrorCode::Success;
    }

    auto res = own_configure(*srv_);
    if (res != ErrorCode::Success) {
        return res;
    }
    
    spdlog::debug("Applying server defaults...");

    // Enable CORS for frontend development
    srv_->set_pre_routing_handler([](const Request& req, Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        
        if (req.method == "OPTIONS") {
            res.status = 200;
            return httplib::Server::HandlerResponse::Handled;
        }
        return httplib::Server::HandlerResponse::Unhandled;
    });

    // Catch-all static handler
    srv_->Get(R"(/.*)", [&](const httplib::Request& req, httplib::Response& res) {
        std::string path = req.path;
        if (path == "/" || path.empty()) {
            path = "index.html";               // SPA entry
        } else if (!path.empty() && path.front() == '/') {
            path.erase(0, 1);                  // strip leading '/'
        }

        if (auto data = get_www_asset<std::string>(path)) {
            res.set_content(*data, resolve_content_type(path));
        } else {
            res.status = 404;
            res.set_content("File not found", "text/plain");
        }
    });

    srv_->set_logger([](const httplib::Request& req, const httplib::Response& res) {
        spdlog::info("{} {} -> {} {}", req.method, req.path, res.status, res.reason);
    });

    is_configured_ = true;
    return ErrorCode::Success;
}

} // namespace dbr
