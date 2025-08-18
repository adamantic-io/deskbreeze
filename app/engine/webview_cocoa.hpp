#pragma once
#ifdef __APPLE__

#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>
#include "ipc_handler.hpp"
#include <spdlog/spdlog.h>

static dbr::ipc::IPCHandlerRegistry* g_ipc_registry_cocoa = nullptr;
static WKWebView* g_webview_cocoa = nullptr;

@interface ScriptMessageHandler : NSObject <WKScriptMessageHandler>
@end

@implementation ScriptMessageHandler
- (void)userContentController:(WKUserContentController *)userContentController 
      didReceiveScriptMessage:(WKScriptMessage *)message {
    if (!g_ipc_registry_cocoa) return;
    
    NSString* messageBody = (NSString*)message.body;
    std::string json_str = [messageBody UTF8String];
    
    if (spdlog::should_log(spdlog::level::debug)) {
        spdlog::debug("Received IPC message: {}", json_str);
    }
    
    dbr::ipc::IPCResponse response = g_ipc_registry_cocoa->handle_json_message(json_str);
    
    if (response.id.empty()) {
        spdlog::warn("IPC response has no ID, cannot handle response properly");
    }
    if (spdlog::should_log(spdlog::level::debug)) {
        spdlog::debug("IPC response: {}", response.to_json().dump());
    }
    
    std::string response_json = response.to_json().dump();
    std::string js_code = "window.dbr.ipc._handleResponse(" + response_json + ");";
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [g_webview_cocoa evaluateJavaScript:[NSString stringWithUTF8String:js_code.c_str()] 
                           completionHandler:nil];
    });
}
@end

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (strong, nonatomic) NSWindow *window;
@property (strong, nonatomic) WKWebView *webView;
@property (strong, nonatomic) ScriptMessageHandler *messageHandler;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    // Create window
    NSRect frame = NSMakeRect(0, 0, 1024, 768);
    self.window = [[NSWindow alloc] initWithContentRect:frame
                                              styleMask:NSWindowStyleMaskTitled | 
                                                       NSWindowStyleMaskClosable | 
                                                       NSWindowStyleMaskResizable
                                                backing:NSBackingStoreBuffered
                                                  defer:NO];
    [self.window setTitle:@"Pet Store App"];
    [self.window center];
    
    // Create WebView
    WKWebViewConfiguration *config = [[WKWebViewConfiguration alloc] init];
    
    // Setup IPC if registry is available
    if (g_ipc_registry_cocoa) {
        self.messageHandler = [[ScriptMessageHandler alloc] init];
        [config.userContentController addScriptMessageHandler:self.messageHandler name:@"deskbreeze"];
        
        // Enable developer tools
        [config.preferences setValue:@YES forKey:@"developerExtrasEnabled"];
    }
    
    self.webView = [[WKWebView alloc] initWithFrame:frame configuration:config];
    g_webview_cocoa = self.webView;
    
    // Load URL
    NSURL *url = [NSURL URLWithString:@"http://localhost:3001"];
    NSURLRequest *request = [NSURLRequest requestWithURL:url];
    [self.webView loadRequest:request];
    
    // Set WebView as window content
    [self.window setContentView:self.webView];
    [self.window makeKeyAndOrderFront:nil];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

@end

inline void send_message_to_webview_cocoa(const dbr::ipc::IPCMessage& msg) {
    if (!g_webview_cocoa || !g_ipc_registry_cocoa) return;

    nlohmann::json message = msg.to_json();
    std::string json_str = message.dump();
    std::string js_code = "window.dbr.ipc._handleMessage(" + json_str + ");";

    dispatch_async(dispatch_get_main_queue(), ^{
        [g_webview_cocoa evaluateJavaScript:[NSString stringWithUTF8String:js_code.c_str()] 
                           completionHandler:nil];
    });
}

inline int webview_cocoa_main(dbr::ipc::IPCHandlerRegistry* ipc_registry = nullptr) {
    @autoreleasepool {
        g_ipc_registry_cocoa = ipc_registry;
        
        NSApplication *app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];
        
        AppDelegate *delegate = [[AppDelegate alloc] init];
        [app setDelegate:delegate];
        
        [app run];
    }
    return 0;
}

#endif