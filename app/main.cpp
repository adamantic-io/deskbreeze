#include <memory>
#include <thread>
#include <chrono>
#include "engine/http_server.hpp"
#include "engine/ipc_handler.hpp"
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

void setup_ipc_handlers(dbr::ipc::IPCHandlerRegistry& registry) {
    // System info handler
    registry.register_handler("system.info", [](const dbr::ipc::IPCMessage& msg) -> dbr::ipc::IPCResponse {
        nlohmann::json info;
        info["platform"] = 
#ifdef __linux__
            "linux";
#elif __APPLE__
            "macos";
#elif _WIN32
            "windows";
#else
            "unknown";
#endif
        info["app"] = "DeskBreeze";
        info["version"] = "1.0.0";
        return dbr::ipc::IPCResponse(info, msg.id);
    });
    
    // Application version
    registry.register_handler("app.version", [](const dbr::ipc::IPCMessage& msg) -> dbr::ipc::IPCResponse {
        return dbr::ipc::IPCResponse(static_cast<std::string>("1.0.0"), msg.id);
    });
    
    // Echo handler for testing
    registry.register_handler("echo", [](const dbr::ipc::IPCMessage& msg) -> dbr::ipc::IPCResponse {
        return dbr::ipc::IPCResponse(msg.params, msg.id);
    });
    
    // Notification handler (just logs for now)
    registry.register_handler("notification.show", [](const dbr::ipc::IPCMessage& msg) -> dbr::ipc::IPCResponse {
        if (msg.params.contains("title") && msg.params.contains("message")) {
            spdlog::info("Notification: {} - {}", msg.params["title"].get<std::string>(), 
                        msg.params["message"].get<std::string>());
        }
        return dbr::ipc::IPCResponse(true, msg.id);
    });
    
    // Window control handlers (placeholders)
    registry.register_handler("window.minimize", [](const dbr::ipc::IPCMessage& msg) -> dbr::ipc::IPCResponse {
        spdlog::info("Window minimize requested");
        return dbr::ipc::IPCResponse(true, msg.id);
    });
    
    registry.register_handler("window.maximize", [](const dbr::ipc::IPCMessage& msg) -> dbr::ipc::IPCResponse {
        spdlog::info("Window maximize requested");
        return dbr::ipc::IPCResponse(true, msg.id);
    });
    
    registry.register_handler("window.close", [](const dbr::ipc::IPCMessage& msg) -> dbr::ipc::IPCResponse {
        spdlog::info("Window close requested");
        return dbr::ipc::IPCResponse(true, msg.id);
    });
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

    // Setup IPC
    dbr::ipc::IPCHandlerRegistry ipc_registry;
    setup_ipc_handlers(ipc_registry);
    spdlog::info("IPC handlers configured");

#ifdef __linux__
    return webview_gtk_main(argc, argv, &ipc_registry);
#elif __APPLE__
    return webview_cocoa_main(&ipc_registry);
#elif _WIN32
    return webview_windows_main(&ipc_registry);
#endif
}
