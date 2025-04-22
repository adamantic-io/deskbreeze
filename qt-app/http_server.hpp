
#pragma once
#include "httplib.h"
#include "sqlite3.h"
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

inline void start_server() {
    using namespace httplib;
    Server svr;

    sqlite3* db;
    sqlite3_open("settings.db", &db);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS settings (key TEXT PRIMARY KEY, value TEXT);", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "INSERT OR IGNORE INTO settings (key, value) VALUES ('username', 'johndoe');", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "INSERT OR IGNORE INTO settings (key, value) VALUES ('language', 'it');", nullptr, nullptr, nullptr);

    svr.Get("/api/settings", [db](const Request&, Response& res) {
        sqlite3_stmt* stmt;
        json j;
        if (sqlite3_prepare_v2(db, "SELECT key, value FROM settings;", -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                std::string key = (const char*)sqlite3_column_text(stmt, 0);
                std::string value = (const char*)sqlite3_column_text(stmt, 1);
                j[key] = value;
            }
        }
        sqlite3_finalize(stmt);
        res.set_content(j.dump(), "application/json");
    });

    svr.Post("/api/settings", [db](const Request& req, Response& res) {
        auto j = json::parse(req.body);
        for (auto& [key, val] : j.items()) {
            sqlite3_stmt* stmt;
            sqlite3_prepare_v2(db, "REPLACE INTO settings (key, value) VALUES (?, ?);", -1, &stmt, nullptr);
            sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, val.dump().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
        res.set_content("ok", "text/plain");
    });

    svr.Get("/", [&](const Request&, Response& res) {
        extern const unsigned char index_html_start[], index_html_end[];
        res.set_content(std::string((char*)index_html_start, index_html_end - index_html_start), "text/html");
    });

    svr.Get("/main.js", [&](const Request&, Response& res) {
        extern const unsigned char main_js_start[], main_js_end[];
        res.set_content(std::string((char*)main_js_start, main_js_end - main_js_start), "application/javascript");
    });

    svr.listen("localhost", 3001);
}
