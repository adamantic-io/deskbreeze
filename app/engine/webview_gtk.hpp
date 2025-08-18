#pragma once
#ifdef __linux__

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <iostream>
#include <spdlog/spdlog.h>
#include "ipc_handler.hpp"

static dbr::ipc::IPCHandlerRegistry* g_ipc_registry = nullptr;
static WebKitWebView* g_webview = nullptr;

static void message_received_callback(WebKitUserContentManager* manager, 
                                    WebKitJavascriptResult* js_result, 
                                    gpointer user_data) {
    if (!g_ipc_registry) return;
    
    JSCValue* value = webkit_javascript_result_get_js_value(js_result);
    if (jsc_value_is_string(value)) {
        char* str_value = jsc_value_to_string(value);
        if (spdlog::should_log(spdlog::level::debug)) {
            spdlog::debug("Received IPC message: {}", str_value);
        }

        dbr::ipc::IPCResponse response = g_ipc_registry->handle_json_message(str_value);

        if (response.id.empty()) {
            spdlog::warn("IPC response has no ID, cannot handle response properly");
        }
        if (spdlog::should_log(spdlog::level::debug)) {
            spdlog::debug("IPC response: {}", response.to_json().dump());
        }
        std::string response_json = response.to_json().dump();
        std::string js_code = "window.dbr.ipc._handleResponse(" + response_json + ");";
        
        webkit_web_view_evaluate_javascript(g_webview, js_code.c_str(), -1, NULL, NULL, NULL, NULL, NULL);
        
        g_free(str_value);
    }
    webkit_javascript_result_unref(js_result);
}

inline void send_message_to_webview(const dbr::ipc::IPCMessage& msg) {
    if (!g_webview || !g_ipc_registry) return;

    nlohmann::json message = msg.to_json();

    std::string json_str = message.dump();
    std::string js_code = "window.dbr.ipc._handleMessage(" + json_str + ");";

    webkit_web_view_evaluate_javascript(g_webview, js_code.c_str(), -1, NULL, NULL, NULL, NULL, NULL);
}

inline void setup_ipc_bridge(WebKitWebView* webview, dbr::ipc::IPCHandlerRegistry* registry) {
    g_ipc_registry = registry;
    g_webview = webview;
    
    WebKitUserContentManager* content_manager = webkit_web_view_get_user_content_manager(webview);
    
    g_signal_connect(content_manager, "script-message-received::deskbreeze",
                     G_CALLBACK(message_received_callback), NULL);
    
    webkit_user_content_manager_register_script_message_handler(content_manager, "deskbreeze");
    
}

inline int webview_gtk_main(int argc, char *argv[], dbr::ipc::IPCHandlerRegistry* ipc_registry = nullptr) {
    gtk_init(&argc, &argv);

    // Create window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Pet Store App");
    gtk_window_set_default_size(GTK_WINDOW(window), 1024, 768);
    
    // Create WebView
    WebKitWebView *webview = WEBKIT_WEB_VIEW(webkit_web_view_new());
    
    // Enable developer tools
    WebKitSettings *settings = webkit_web_view_get_settings(webview);
    webkit_settings_set_enable_developer_extras(settings, TRUE);
        
    webkit_web_view_load_uri(webview, "http://localhost:3001");

    // Setup IPC bridge if registry provided
    if (ipc_registry) {
        setup_ipc_bridge(webview, ipc_registry);
    }

    // Add WebView to window
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(webview));
    
    // Connect close signal
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    // Show window
    gtk_widget_show_all(window);
    
    // Run main loop
    gtk_main();
    
    return 0;
}

#endif