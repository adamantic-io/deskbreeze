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

int main(int argc, char *argv[]) {
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