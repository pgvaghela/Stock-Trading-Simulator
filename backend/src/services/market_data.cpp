#include "market_data.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <stdexcept>

// HTTPS client (CPPHTTPLIB_OPENSSL_SUPPORT defined via CMake)
#include "httplib.h"

// Crow's built-in JSON for building/parsing JSON payloads
#include "crow.h"

MarketDataService::MarketDataService(std::string api_key, std::string api_base_url,
                                     BroadcastFn broadcast)
    : api_key_(std::move(api_key))
    , api_base_url_(std::move(api_base_url))
    , broadcast_(std::move(broadcast)) {}

MarketDataService::~MarketDataService() { stop(); }

void MarketDataService::start() {
    running_ = true;
    bg_thread_ = std::thread(&MarketDataService::run_loop, this);
}

void MarketDataService::stop() {
    running_ = false;
    if (bg_thread_.joinable()) bg_thread_.join();
}

void MarketDataService::run_loop() {
    while (running_) {
        for (const auto& symbol : symbols_) {
            if (!running_) return;
            try {
                double price = fetch_from_api(symbol);
                if (price <= 0.0) price = mock_price(symbol);

                // Refresh cache
                {
                    std::lock_guard<std::mutex> lock(cache_mtx_);
                    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count();
                    cache_[symbol] = {price, now_ms + 300'000LL};
                }

                // Build ISO-8601 timestamp
                auto now = std::chrono::system_clock::now();
                std::time_t t = std::chrono::system_clock::to_time_t(now);
                char ts[32];
                std::strftime(ts, sizeof(ts), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&t));

                // Broadcast {symbol, price, timestamp} to all WS clients
                crow::json::wvalue msg;
                msg["symbol"]    = symbol;
                msg["price"]     = price;
                msg["timestamp"] = std::string(ts);
                broadcast_(msg.dump());

            } catch (const std::exception& e) {
                std::cerr << "[MarketData] " << symbol << ": " << e.what() << std::endl;
            }
            // Small pause between tickers to respect API rate limits
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        // Wait ~60 s before next full refresh cycle
        for (int i = 0; i < 600 && running_; ++i)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

double MarketDataService::fetch_from_api(const std::string& symbol) {
    try {
        httplib::SSLClient cli("www.alphavantage.co");
        cli.set_connection_timeout(5);
        cli.set_read_timeout(10);

        std::string path = "/query?function=GLOBAL_QUOTE&symbol=" + symbol
                         + "&apikey=" + api_key_;
        auto res = cli.Get(path);
        if (!res || res->status != 200) return 0.0;

        // Parse response with Crow's JSON
        auto j = crow::json::load(res->body);
        if (!j)                            return 0.0;
        if (j.has("Note"))                 return 0.0;
        if (j.has("Error Message"))        return 0.0;
        if (!j.has("Global Quote"))        return 0.0;

        const auto& quote = j["Global Quote"];
        if (!quote.has("05. price"))       return 0.0;

        return std::stod(std::string(quote["05. price"].s()));
    } catch (...) {
        return 0.0;
    }
}

double MarketDataService::get_price(const std::string& symbol) {
    // Serve from cache if still fresh
    {
        std::lock_guard<std::mutex> lock(cache_mtx_);
        auto it = cache_.find(symbol);
        if (it != cache_.end()) {
            auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            if (now_ms < it->second.expires_ms)
                return it->second.price;
        }
    }

    double price = fetch_from_api(symbol);
    if (price <= 0.0) price = mock_price(symbol);

    {
        std::lock_guard<std::mutex> lock(cache_mtx_);
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        cache_[symbol] = {price, now_ms + 300'000LL};
    }
    return price;
}

double MarketDataService::mock_price(const std::string& symbol) {
    static const std::unordered_map<std::string, double> prices{
        {"AAPL", 213.88}, {"GOOG", 194.08}, {"MSFT", 415.22},
        {"TSLA", 248.50}, {"AMZN", 178.12}
    };
    auto it = prices.find(symbol);
    return it != prices.end() ? it->second : 100.0;
}
