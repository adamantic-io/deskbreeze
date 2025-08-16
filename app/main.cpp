#include <thread>
#include <chrono>
#include "http_server.hpp"

#ifdef __linux__
#include "webview_gtk.hpp"
#elif __APPLE__
#include "webview_cocoa.hpp"
#elif _WIN32
#include "webview_windows.hpp"
#endif


#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#
inline void init_logging() {
    auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto logger  = std::make_shared<spdlog::logger>("app", spdlog::sinks_init_list{console});
    spdlog::set_default_logger(logger);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    spdlog::set_level(spdlog::level::debug); // overridden by SPDLOG_ACTIVE_LEVEL at compile-time
}

int main(int argc, char *argv[]) {
    init_logging();
    spdlog::info("Starting DeskBreeze WebView application...");
    // Start HTTP server in background thread
    std::thread server_thread([]() {
        start_server();
    });
    server_thread.detach();

    // Give server time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Create and show WebView
#ifdef __linux__
    return webview_gtk_main(argc, argv);
#elif __APPLE__
    return webview_cocoa_main();
#elif _WIN32
    return webview_windows_main();
#endif
    
    return 0;
}