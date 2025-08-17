#include <memory>
#include <thread>
#include <chrono>
#include "engine/http_server.hpp"
#include "common_defs.hpp"
#include "own_server.hpp"

#ifdef __linux__
#include "engine/webview_gtk.hpp"
#elif __APPLE__
#include "engine/webview_cocoa.hpp"
#elif _WIN32
#include "engine/webview_windows.hpp"
#endif


#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

using namespace dbr;
using namespace std;

inline void init_logging() {
    auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto logger  = std::make_shared<spdlog::logger>("app", spdlog::sinks_init_list{console});
    spdlog::set_default_logger(logger);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    spdlog::set_level(spdlog::level::debug); // overridden by SPDLOG_ACTIVE_LEVEL at compile-time
}

std::unique_ptr<dbr::Server> make_server() {
    return std::make_unique<OwnServer>();
}

int main(int argc, char* argv[]) {
    init_logging();
    spdlog::info("Starting DeskBreeze WebView application...");

    auto server = make_server();
    if (!server) {
        spdlog::error("Failed to create server instance");
        return 1;
    }
    spdlog::debug("Configuring server...");
    if (! (server->configure()) ) {
        spdlog::error("Failed to configure server");
        return 1;
    }
    spdlog::debug("Server configured successfully, starting server...");
    if (!server->start()) {
        spdlog::error("Failed to start server");
        return 1;
    }
    server->wait_until_ready();
    spdlog::debug("Server started successfully, proceeding to create WebView...");

#ifdef __linux__
    return webview_gtk_main(argc, argv);
#elif __APPLE__
    return webview_cocoa_main();
#elif _WIN32
    return webview_windows_main();
#endif
}
