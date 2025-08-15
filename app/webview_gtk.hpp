#pragma once
#ifdef __linux__

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

inline int webview_gtk_main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Create window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Pet Store App");
    gtk_window_set_default_size(GTK_WINDOW(window), 1024, 768);
    
    // Create WebView
    WebKitWebView *webview = WEBKIT_WEB_VIEW(webkit_web_view_new());
    webkit_web_view_load_uri(webview, "http://localhost:3001");
    
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