#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>
#include <unordered_map>

// Callback used to push a JSON price-update string to all WebSocket clients.
using BroadcastFn = std::function<void(const std::string&)>;

class MarketDataService {
public:
    MarketDataService(std::string api_key, std::string api_base_url,
                      BroadcastFn broadcast);
    ~MarketDataService();

    // Start the 60-second polling loop in a background thread.
    void start();
    void stop();

    // Synchronous single-stock price lookup with 5-minute cache.
    // Falls back to mock prices on API failure.
    double get_price(const std::string& symbol);

private:
    void   run_loop();
    double fetch_from_api(const std::string& symbol);
    static double mock_price(const std::string& symbol);

    std::string  api_key_;
    std::string  api_base_url_;
    BroadcastFn  broadcast_;

    const std::vector<std::string> symbols_{"AAPL", "GOOG", "MSFT", "TSLA", "AMZN"};

    struct CachedPrice {
        double    price;
        long long expires_ms;
    };
    std::unordered_map<std::string, CachedPrice> cache_;
    std::mutex cache_mtx_;

    std::thread       bg_thread_;
    std::atomic<bool> running_{false};
};
