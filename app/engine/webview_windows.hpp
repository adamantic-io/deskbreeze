#pragma once
#ifdef _WIN32

#include <windows.h>
#include <wrl.h>
#include <wil/com.h>
#include "WebView2.h"
#include "ipc_handler.hpp"
#include <spdlog/spdlog.h>
#include <string>

using namespace Microsoft::WRL;

static dbr::ipc::IPCHandlerRegistry* g_ipc_registry_windows = nullptr;
static ICoreWebView2* g_webview_windows = nullptr;

class WebViewApp {
private:
    HWND hwnd_;
    wil::com_ptr<ICoreWebView2Controller> webViewController_;
    wil::com_ptr<ICoreWebView2> webView_;

    void SetupIPC() {
        if (!g_ipc_registry_windows || !webView_) return;

        // Add support for window.webkit.messageHandlers.deskbreeze
        webView_->AddScriptToExecuteOnDocumentCreated(
            LR"(
            window.webkit = window.webkit || {};
            window.webkit.messageHandlers = window.webkit.messageHandlers || {};
            window.webkit.messageHandlers.deskbreeze = {
                postMessage: function(message) {
                    window.chrome.webview.postMessage(message);
                }
            };
            )",
            nullptr
        );

        // Set up message handler for IPC
        EventRegistrationToken token;
        webView_->add_WebMessageReceived(
            Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                [this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                    if (!g_ipc_registry_windows) return S_OK;

                    wil::unique_cotaskmem_string messageRaw;
                    args->TryGetWebMessageAsString(&messageRaw);
                    
                    std::string json_str;
                    
                    // Convert wide string to UTF-8
                    int len = WideCharToMultiByte(CP_UTF8, 0, messageRaw.get(), -1, nullptr, 0, nullptr, nullptr);
                    if (len > 0) {
                        std::vector<char> buffer(len);
                        WideCharToMultiByte(CP_UTF8, 0, messageRaw.get(), -1, buffer.data(), len, nullptr, nullptr);
                        json_str = std::string(buffer.data());
                    }
                    
                    if (spdlog::should_log(spdlog::level::debug)) {
                        spdlog::debug("Received IPC message: {}", json_str);
                    }
                    
                    dbr::ipc::IPCResponse response = g_ipc_registry_windows->handle_json_message(json_str);
                    
                    if (response.id.empty()) {
                        spdlog::warn("IPC response has no ID, cannot handle response properly");
                    }
                    if (spdlog::should_log(spdlog::level::debug)) {
                        spdlog::debug("IPC response: {}", response.to_json().dump());
                    }
                    
                    std::string response_json = response.to_json().dump();
                    std::string js_code = "window.dbr.ipc._handleResponse(" + response_json + ");";
                    
                    // Convert UTF-8 to wide string
                    int wlen = MultiByteToWideChar(CP_UTF8, 0, js_code.c_str(), -1, nullptr, 0);
                    if (wlen > 0) {
                        std::vector<wchar_t> wbuffer(wlen);
                        MultiByteToWideChar(CP_UTF8, 0, js_code.c_str(), -1, wbuffer.data(), wlen);
                        webView_->ExecuteScript(wbuffer.data(), nullptr);
                    }
                    
                    return S_OK;
                }
            ).Get(), &token);
    }

public:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        WebViewApp* app = reinterpret_cast<WebViewApp*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        
        switch (msg) {
        case WM_SIZE:
            if (app && app->webViewController_) {
                RECT bounds;
                GetClientRect(hwnd, &bounds);
                app->webViewController_->put_Bounds(bounds);
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        return 0;
    }

    int Run() {
        // Register window class
        WNDCLASS wc = {};
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = L"WebViewWindow";
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

        RegisterClass(&wc);

        // Create window
        hwnd_ = CreateWindowEx(
            0,
            L"WebViewWindow",
            L"Pet Store App",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768,
            nullptr, nullptr, GetModuleHandle(nullptr), nullptr
        );

        SetWindowLongPtr(hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

        // Create WebView
        CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
            Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
                [this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                    env->CreateCoreWebView2Controller(hwnd_,
                        Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                            [this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                                webViewController_ = controller;
                                webViewController_->get_CoreWebView2(&webView_);
                                g_webview_windows = webView_.get();

                                // Enable developer tools if IPC is available
                                if (g_ipc_registry_windows) {
                                    wil::com_ptr<ICoreWebView2Settings> settings;
                                    webView_->get_Settings(&settings);
                                    settings->put_AreDevToolsEnabled(TRUE);
                                    
                                    // Setup IPC bridge
                                    SetupIPC();
                                }

                                // Set bounds
                                RECT bounds;
                                GetClientRect(hwnd_, &bounds);
                                webViewController_->put_Bounds(bounds);

                                // Navigate to URL
                                webView_->Navigate(L"http://localhost:3001");

                                return S_OK;
                            }).Get());
                    return S_OK;
                }).Get());

        ShowWindow(hwnd_, SW_SHOWDEFAULT);
        UpdateWindow(hwnd_);

        // Message loop
        MSG msg = {};
        while (GetMessage(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        return 0;
    }
};

inline void send_message_to_webview_windows(const dbr::ipc::IPCMessage& msg) {
    if (!g_webview_windows || !g_ipc_registry_windows) return;

    nlohmann::json message = msg.to_json();
    std::string json_str = message.dump();
    std::string js_code = "window.dbr.ipc._handleMessage(" + json_str + ");";

    // Convert UTF-8 to wide string
    int wlen = MultiByteToWideChar(CP_UTF8, 0, js_code.c_str(), -1, nullptr, 0);
    if (wlen > 0) {
        std::vector<wchar_t> wbuffer(wlen);
        MultiByteToWideChar(CP_UTF8, 0, js_code.c_str(), -1, wbuffer.data(), wlen);
        g_webview_windows->ExecuteScript(wbuffer.data(), nullptr);
    }
}

inline int webview_windows_main(dbr::ipc::IPCHandlerRegistry* ipc_registry = nullptr) {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    
    g_ipc_registry_windows = ipc_registry;
    
    WebViewApp app;
    int result = app.Run();
    CoUninitialize();
    return result;
}

#endif