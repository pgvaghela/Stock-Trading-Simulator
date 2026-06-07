#pragma once
#include "metrics.h"
#include <string>
#include <cstdint>
#include <queue>
#include <mutex>
#include <memory>
#include <functional>

// Mirrors Java's OrderMatchingService with a buy max-heap and sell min-heap.
class OrderMatchingService {
public:
    enum class Side { BUY, SELL };

    struct MatchingOrder {
        int64_t     portfolio_id;
        std::string symbol;
        Side        side;
        int         quantity;
        double      price;
    };

    explicit OrderMatchingService(std::shared_ptr<MetricsService> metrics);

    // Thread-safe: place an order and immediately attempt to match it.
    void place_order(MatchingOrder order);

private:
    // Buy book: max-heap by price (highest bid at top)
    struct BuyCompare {
        bool operator()(const MatchingOrder& a, const MatchingOrder& b) const {
            return a.price < b.price;
        }
    };
    // Sell book: min-heap by price (lowest ask at top)
    struct SellCompare {
        bool operator()(const MatchingOrder& a, const MatchingOrder& b) const {
            return a.price > b.price;
        }
    };

    std::priority_queue<MatchingOrder, std::vector<MatchingOrder>, BuyCompare>  buy_book_;
    std::priority_queue<MatchingOrder, std::vector<MatchingOrder>, SellCompare> sell_book_;

    std::mutex                       mtx_;
    std::shared_ptr<MetricsService>  metrics_;

    void execute_trade(const MatchingOrder& incoming, const MatchingOrder& book_order,
                       int qty, double price);
    double calculate_portfolio_value(int64_t portfolio_id);
};
