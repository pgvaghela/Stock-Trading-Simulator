#include "crow.h"
#include "database.h"
#include "repositories.h"
#include "services/order_matching.h"
#include "services/market_data.h"
#include "services/metrics.h"
#include "routes/users.h"
#include "routes/portfolios.h"
#include "routes/stocks.h"
#include "routes/orders.h"
#include "routes/value_history.h"

#include <set>
#include <mutex>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstdlib>
#include <iostream>

// ---------- Config helpers ----------

static std::string env(const char* key, const char* fallback = "") {
    const char* v = std::getenv(key);
    return v ? v : fallback;
}

static int env_int(const char* key, int fallback) {
    const char* v = std::getenv(key);
    return v ? std::stoi(v) : fallback;
}

// ---------- WebSocket connection hub ----------

struct WsHub {
    std::set<crow::websocket::connection*> conns;
    std::mutex mtx;

    void add(crow::websocket::connection* c) {
        std::lock_guard lock(mtx);
        conns.insert(c);
    }
    void remove(crow::websocket::connection* c) {
        std::lock_guard lock(mtx);
        conns.erase(c);
    }
    void broadcast(const std::string& msg) {
        std::lock_guard lock(mtx);
        for (auto* c : conns) {
            try { c->send_text(msg); } catch (...) {}
        }
    }
};

// ---------- Static file helpers ----------

static std::string mime_type(const std::string& path) {
    auto ext = std::filesystem::path(path).extension().string();
    if (ext == ".html") return "text/html";
    if (ext == ".js")   return "application/javascript";
    if (ext == ".css")  return "text/css";
    if (ext == ".json") return "application/json";
    if (ext == ".png")  return "image/png";
    if (ext == ".svg")  return "image/svg+xml";
    if (ext == ".ico")  return "image/x-icon";
    return "application/octet-stream";
}

static std::string read_file(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
}

// ---------- main ----------

int main() {
    // Configuration via environment variables
    const std::string db_path    = env("DB_PATH",       "stock_simulator.db");
    const std::string api_key    = env("MARKET_API_KEY", "demo");
    const std::string api_url    = env("MARKET_API_URL", "https://www.alphavantage.co");
    const std::string static_dir = env("STATIC_DIR",
        "../demo/src/main/resources/static/frontend/build");
    const int port               = env_int("PORT", 8080);

    // Database
    init_db(db_path);
    seed_db();
    std::cout << "[Startup] Database ready at " << db_path << std::endl;

    // Services
    auto metrics  = std::make_shared<MetricsService>();
    auto matching = std::make_shared<OrderMatchingService>(metrics);

    auto hub = std::make_shared<WsHub>();
    auto market = std::make_shared<MarketDataService>(
        api_key, api_url,
        [hub](const std::string& msg) { hub->broadcast(msg); }
    );
    market->start();
    std::cout << "[Startup] Market data service started" << std::endl;

    // Crow app
    crow::SimpleApp app;
    app.loglevel(crow::LogLevel::Info);

    // ---- WebSocket endpoint ----
    CROW_WEBSOCKET_ROUTE(app, "/ws")
        .onopen([hub](crow::websocket::connection& conn) {
            hub->add(&conn);
            std::cout << "[WS] Client connected" << std::endl;
        })
        .onclose([hub](crow::websocket::connection& conn, const std::string&) {
            hub->remove(&conn);
            std::cout << "[WS] Client disconnected" << std::endl;
        })
        .onmessage([](crow::websocket::connection& /*conn*/,
                      const std::string& /*data*/, bool /*is_binary*/) {
            // Clients only receive; they don't send.
        });

    // ---- REST routes ----
    register_user_routes(app);
    register_portfolio_routes(app);
    register_stock_routes(app, market);
    register_order_routes(app, market);
    register_value_history_routes(app, market);

    // ---- Static files (React SPA) ----
    CROW_CATCHALL_ROUTE(app)([&static_dir](const crow::request& req) {
        std::string url_path = req.url;
        // Strip query string
        auto qpos = url_path.find('?');
        if (qpos != std::string::npos) url_path = url_path.substr(0, qpos);

        // Attempt to serve the exact file
        std::string file_path = static_dir + url_path;
        if (std::filesystem::is_regular_file(file_path)) {
            crow::response res(read_file(file_path));
            res.set_header("Content-Type", mime_type(file_path));
            return res;
        }

        // SPA fallback — serve index.html for any unmatched route
        std::string index_path = static_dir + "/index.html";
        if (std::filesystem::is_regular_file(index_path)) {
            crow::response res(read_file(index_path));
            res.set_header("Content-Type", "text/html");
            return res;
        }

        return crow::response(404, "Not found");
    });

    std::cout << "[Startup] Listening on port " << port << std::endl;
    app.port(port).multithreaded().run();

    market->stop();
    return 0;
}
