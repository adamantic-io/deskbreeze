
#include <QApplication>
#include <QWebEngineView>
#include <thread>
#include "http_server.hpp"

int main(int argc, char *argv[]) {
    std::thread server_thread([]() {
        start_server();
    });

    QApplication app(argc, argv);
    QWebEngineView view;
    view.load(QUrl("http://localhost:3001"));
    view.resize(1024, 768);
    view.show();

    int result = app.exec();
    server_thread.detach();
    return result;
}
