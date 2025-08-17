#pragma once
#ifdef _WIN32

#include <windows.h>
#include <wrl.h>
#include <wil/com.h>
#include "WebView2.h"

using namespace Microsoft::WRL;

class WebViewApp {
private:
    HWND hwnd_;
    wil::com_ptr<ICoreWebView2Controller> webViewController_;
    wil::com_ptr<ICoreWebView2> webView_;

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

inline int webview_windows_main() {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    WebViewApp app;
    int result = app.Run();
    CoUninitialize();
    return result;
}

#endif