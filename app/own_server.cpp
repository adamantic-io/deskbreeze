#include "own_server.hpp"
#include "common_defs.hpp"
#include <sqlite3.h>
#include <nlohmann/json.hpp>
#include <memory>
#include <spdlog/spdlog.h>

using json = nlohmann::json;


void init_database(std::shared_ptr<sqlite3> db) {
    spdlog::debug("Initializing database...");
    // Create pets table
    sqlite3_exec(db.get(), R"(
        CREATE TABLE IF NOT EXISTS pets (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            species TEXT NOT NULL,
            breed TEXT,
            age INTEGER,
            price REAL NOT NULL,
            description TEXT,
            image_url TEXT,
            available BOOLEAN DEFAULT 1,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )", nullptr, nullptr, nullptr);

    // Create categories table
    sqlite3_exec(db.get(), R"(
        CREATE TABLE IF NOT EXISTS categories (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            description TEXT
        );
    )", nullptr, nullptr, nullptr);

    // Create pet_categories junction table
    sqlite3_exec(db.get(), R"(
        CREATE TABLE IF NOT EXISTS pet_categories (
            pet_id INTEGER,
            category_id INTEGER,
            PRIMARY KEY (pet_id, category_id),
            FOREIGN KEY (pet_id) REFERENCES pets(id),
            FOREIGN KEY (category_id) REFERENCES categories(id)
        );
    )", nullptr, nullptr, nullptr);

    // Create orders table
    sqlite3_exec(db.get(), R"(
        CREATE TABLE IF NOT EXISTS orders (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            customer_name TEXT NOT NULL,
            customer_email TEXT NOT NULL,
            customer_phone TEXT,
            total_amount REAL NOT NULL,
            status TEXT DEFAULT 'pending',
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )", nullptr, nullptr, nullptr);

    // Create order_items table
    sqlite3_exec(db.get(), R"(
        CREATE TABLE IF NOT EXISTS order_items (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            order_id INTEGER,
            pet_id INTEGER,
            quantity INTEGER DEFAULT 1,
            price REAL NOT NULL,
            FOREIGN KEY (order_id) REFERENCES orders(id),
            FOREIGN KEY (pet_id) REFERENCES pets(id)
        );
    )", nullptr, nullptr, nullptr);

    // Insert sample categories
    sqlite3_exec(db.get(), R"(
        INSERT OR IGNORE INTO categories (name, description) VALUES 
        ('Dogs', 'Loyal and friendly companions'),
        ('Cats', 'Independent and playful pets'),
        ('Birds', 'Colorful and musical friends'),
        ('Fish', 'Peaceful aquatic companions'),
        ('Small Animals', 'Hamsters, rabbits, and other small pets');
    )", nullptr, nullptr, nullptr);

    // Insert sample pets
    sqlite3_exec(db.get(), R"(
        INSERT OR IGNORE INTO pets (name, species, breed, age, price, description, image_url, available) VALUES 
        ('Buddy', 'Dog', 'Golden Retriever', 2, 800.00, 'Friendly and energetic golden retriever, great with kids', '/images/golden-retriever.svg', 1),
        ('Luna', 'Cat', 'Persian', 1, 600.00, 'Beautiful Persian cat with long silky fur', '/images/persian-cat.svg', 1),
        ('Charlie', 'Dog', 'Labrador', 3, 750.00, 'Well-trained Labrador, perfect family dog', '/images/labrador.svg', 1),
        ('Whiskers', 'Cat', 'Siamese', 2, 550.00, 'Elegant Siamese cat with striking blue eyes', '/images/siamese-cat.svg', 1),
        ('Tweety', 'Bird', 'Canary', 1, 150.00, 'Beautiful singing canary with bright yellow feathers', '/images/canary.svg', 1),
        ('Nemo', 'Fish', 'Goldfish', 1, 25.00, 'Healthy goldfish perfect for beginners', '/images/goldfish.svg', 1),
        ('Fluffy', 'Small Animal', 'Rabbit', 1, 120.00, 'Adorable rabbit with soft white fur', '/images/rabbit.svg', 1),
        ('Max', 'Dog', 'German Shepherd', 4, 900.00, 'Intelligent and protective German Shepherd', '/images/german-shepherd.svg', 1),
        ('Mittens', 'Cat', 'Maine Coon', 2, 700.00, 'Large and gentle Maine Coon cat', '/images/maine-coon.svg', 1),
        ('Polly', 'Bird', 'Parrot', 3, 400.00, 'Colorful parrot that can learn to speak', '/images/parrot.svg', 1);
    )", nullptr, nullptr, nullptr);

    // Link pets to categories
    sqlite3_exec(db.get(), R"(
        INSERT OR IGNORE INTO pet_categories (pet_id, category_id) VALUES 
        (1, 1), (3, 1), (8, 1),  -- Dogs
        (2, 2), (4, 2), (9, 2),  -- Cats
        (5, 3), (10, 3),         -- Birds
        (6, 4),                  -- Fish
        (7, 5);                  -- Small Animals
    )", nullptr, nullptr, nullptr);
}



dbr::ErrorCode OwnServer::own_configure(httplib::Server& srv) {
    using namespace httplib;

    // Add custom routes or handlers here
    srv.Get("/api/health", [](const Request& req, Response& res) {
        res.set_content("{\"status\": \"ok\"}", "application/json");
    });
    // Example: Add a route to get server info
    srv.Get("/api/server_info", [](const Request& req, Response& res) {
        json info;
        info["name"] = "DeskBreeze Server";
        info["version"] = "1.0.0";
        res.set_content(info.dump(), "application/json");
    });


    sqlite3* db__;
    sqlite3_open("petstore.db", &db__);
    std::shared_ptr<sqlite3> dbptr{db__, sqlite3_close};
    init_database(dbptr);

    // Get all pets with optional filtering
    srv.Get("/api/pets", [dbptr](const Request& req, Response& res) {
        std::string query = R"(
            SELECT p.*, GROUP_CONCAT(c.name) as categories
            FROM pets p
            LEFT JOIN pet_categories pc ON p.id = pc.pet_id
            LEFT JOIN categories c ON pc.category_id = c.id
            WHERE p.available = 1
        )";
        
        // Add filtering by category
        if (req.has_param("category")) {
            query += " AND c.name = ?";
        }
        
        // Add search by name/breed
        if (req.has_param("search")) {
            query += " AND (p.name LIKE ? OR p.breed LIKE ? OR p.species LIKE ?)";
        }
        
        query += " GROUP BY p.id ORDER BY p.created_at DESC";
        
        sqlite3_stmt* stmt;
        json pets = json::array();
        
        if (sqlite3_prepare_v2(dbptr.get(), query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            int param_idx = 1;
            
            if (req.has_param("category")) {
                sqlite3_bind_text(stmt, param_idx++, req.get_param_value("category").c_str(), -1, SQLITE_TRANSIENT);
            }
            
            if (req.has_param("search")) {
                std::string search = "%" + req.get_param_value("search") + "%";
                sqlite3_bind_text(stmt, param_idx++, search.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, param_idx++, search.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, param_idx++, search.c_str(), -1, SQLITE_TRANSIENT);
            }
            
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                json pet;
                pet["id"] = sqlite3_column_int(stmt, 0);
                pet["name"] = (const char*)sqlite3_column_text(stmt, 1);
                pet["species"] = (const char*)sqlite3_column_text(stmt, 2);
                pet["breed"] = sqlite3_column_text(stmt, 3) ? (const char*)sqlite3_column_text(stmt, 3) : "";
                pet["age"] = sqlite3_column_int(stmt, 4);
                pet["price"] = sqlite3_column_double(stmt, 5);
                pet["description"] = sqlite3_column_text(stmt, 6) ? (const char*)sqlite3_column_text(stmt, 6) : "";
                pet["image_url"] = sqlite3_column_text(stmt, 7) ? (const char*)sqlite3_column_text(stmt, 7) : "";
                pet["available"] = sqlite3_column_int(stmt, 8) == 1;
                pet["created_at"] = (const char*)sqlite3_column_text(stmt, 9);
                pet["categories"] = sqlite3_column_text(stmt, 10) ? (const char*)sqlite3_column_text(stmt, 10) : "";
                pets.push_back(pet);
            }
        }
        sqlite3_finalize(stmt);
        res.set_content(pets.dump(), "application/json");
    });

    // Get single pet by ID
    srv.Get("/api/pets/(\\d+)", [dbptr](const Request& req, Response& res) {
        int pet_id = std::stoi(req.matches[1]);
        sqlite3_stmt* stmt;
        json pet;
        
        const char* query = R"(
            SELECT p.*, GROUP_CONCAT(c.name) as categories
            FROM pets p
            LEFT JOIN pet_categories pc ON p.id = pc.pet_id
            LEFT JOIN categories c ON pc.category_id = c.id
            WHERE p.id = ?
            GROUP BY p.id
        )";
        
        if (sqlite3_prepare_v2(dbptr.get(), query, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, pet_id);
            
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                pet["id"] = sqlite3_column_int(stmt, 0);
                pet["name"] = (const char*)sqlite3_column_text(stmt, 1);
                pet["species"] = (const char*)sqlite3_column_text(stmt, 2);
                pet["breed"] = sqlite3_column_text(stmt, 3) ? (const char*)sqlite3_column_text(stmt, 3) : "";
                pet["age"] = sqlite3_column_int(stmt, 4);
                pet["price"] = sqlite3_column_double(stmt, 5);
                pet["description"] = sqlite3_column_text(stmt, 6) ? (const char*)sqlite3_column_text(stmt, 6) : "";
                pet["image_url"] = sqlite3_column_text(stmt, 7) ? (const char*)sqlite3_column_text(stmt, 7) : "";
                pet["available"] = sqlite3_column_int(stmt, 8) == 1;
                pet["created_at"] = (const char*)sqlite3_column_text(stmt, 9);
                pet["categories"] = sqlite3_column_text(stmt, 10) ? (const char*)sqlite3_column_text(stmt, 10) : "";
            }
        }
        sqlite3_finalize(stmt);
        
        if (pet.empty()) {
            res.status = 404;
            res.set_content("{\"error\": \"Pet not found\"}", "application/json");
        } else {
            res.set_content(pet.dump(), "application/json");
        }
    });

    // Get all categories
    srv.Get("/api/categories", [dbptr](const Request& req, Response& res) {
        sqlite3_stmt* stmt;
        json categories = json::array();
        
        const char* query = "SELECT id, name, description FROM categories ORDER BY name";
        
        if (sqlite3_prepare_v2(dbptr.get(), query, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                json category;
                category["id"] = sqlite3_column_int(stmt, 0);
                category["name"] = (const char*)sqlite3_column_text(stmt, 1);
                category["description"] = sqlite3_column_text(stmt, 2) ? (const char*)sqlite3_column_text(stmt, 2) : "";
                categories.push_back(category);
            }
        }
        sqlite3_finalize(stmt);
        res.set_content(categories.dump(), "application/json");
    });

    // Create new order
    srv.Post("/api/orders", [dbptr](const Request& req, Response& res) {
        auto order_data = json::parse(req.body);
        
        sqlite3_stmt* stmt;
        const char* query = R"(
            INSERT INTO orders (customer_name, customer_email, customer_phone, total_amount, status)
            VALUES (?, ?, ?, ?, 'pending')
        )";
        
        if (sqlite3_prepare_v2(dbptr.get(), query, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, order_data["customer_name"].get<std::string>().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, order_data["customer_email"].get<std::string>().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, order_data["customer_phone"].get<std::string>().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_double(stmt, 4, order_data["total_amount"].get<double>());
            
            if (sqlite3_step(stmt) == SQLITE_DONE) {
                int order_id = sqlite3_last_insert_rowid(dbptr.get());
                
                // Insert order items
                for (auto& item : order_data["items"]) {
                    sqlite3_stmt* item_stmt;
                    const char* item_query = "INSERT INTO order_items (order_id, pet_id, quantity, price) VALUES (?, ?, ?, ?)";
                    
                    if (sqlite3_prepare_v2(dbptr.get(), item_query, -1, &item_stmt, nullptr) == SQLITE_OK) {
                        sqlite3_bind_int(item_stmt, 1, order_id);
                        sqlite3_bind_int(item_stmt, 2, item["pet_id"].get<int>());
                        sqlite3_bind_int(item_stmt, 3, item["quantity"].get<int>());
                        sqlite3_bind_double(item_stmt, 4, item["price"].get<double>());
                        sqlite3_step(item_stmt);
                    }
                    sqlite3_finalize(item_stmt);
                }
                
                json response;
                response["order_id"] = order_id;
                response["status"] = "success";
                res.set_content(response.dump(), "application/json");
            } else {
                res.status = 500;
                res.set_content("{\"error\": \"Failed to create order\"}", "application/json");
            }
        }
        sqlite3_finalize(stmt);
    });

    // Get orders (for admin)
    srv.Get("/api/orders", [dbptr](const Request& req, Response& res) {
        sqlite3_stmt* stmt;
        json orders = json::array();
        
        const char* query = R"(
            SELECT o.*, COUNT(oi.id) as item_count
            FROM orders o
            LEFT JOIN order_items oi ON o.id = oi.order_id
            GROUP BY o.id
            ORDER BY o.created_at DESC
        )";

        if (sqlite3_prepare_v2(dbptr.get(), query, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                json order;
                order["id"] = sqlite3_column_int(stmt, 0);
                order["customer_name"] = (const char*)sqlite3_column_text(stmt, 1);
                order["customer_email"] = (const char*)sqlite3_column_text(stmt, 2);
                order["customer_phone"] = sqlite3_column_text(stmt, 3) ? (const char*)sqlite3_column_text(stmt, 3) : "";
                order["total_amount"] = sqlite3_column_double(stmt, 4);
                order["status"] = (const char*)sqlite3_column_text(stmt, 5);
                order["created_at"] = (const char*)sqlite3_column_text(stmt, 6);
                order["item_count"] = sqlite3_column_int(stmt, 7);
                orders.push_back(order);
            }
        }
        sqlite3_finalize(stmt);
        res.set_content(orders.dump(), "application/json");
    });


    return dbr::ErrorCode::Success;
}
