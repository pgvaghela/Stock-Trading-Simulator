#include "order_matching.h"
#include "../repositories.h"
#include <iostream>
#include <unordered_map>

OrderMatchingService::OrderMatchingService(std::shared_ptr<MetricsService> metrics)
    : metrics_(std::move(metrics)) {}

void OrderMatchingService::place_order(MatchingOrder order) {
    std::lock_guard<std::mutex> lock(mtx_);

    auto& opposite = (order.side == Side::BUY) ? sell_book_ : buy_book_;

    while (order.quantity > 0 && !opposite.empty()) {
        MatchingOrder best = opposite.top();

        bool match = (order.side == Side::BUY)
            ? best.price <= order.price
            : best.price >= order.price;

        if (!match) break;

        int matched_qty = std::min(order.quantity, best.quantity);
        execute_trade(order, best, matched_qty, best.price);

        order.quantity -= matched_qty;
        best.quantity  -= matched_qty;
        opposite.pop();
        // Re-insert a partially-filled book order
        if (best.quantity > 0) opposite.push(best);
    }

    if (order.quantity > 0) {
        if (order.side == Side::BUY) buy_book_.push(order);
        else                         sell_book_.push(order);
    }

    metrics_->publish_order_rate(1);
}

void OrderMatchingService::execute_trade(const MatchingOrder& incoming,
                                         const MatchingOrder& /*book_order*/,
                                         int qty, double price) {
    Transaction t;
    t.portfolio_id = incoming.portfolio_id;
    t.stock_symbol = incoming.symbol;
    t.quantity     = qty;
    t.price        = price;
    t.type         = (incoming.side == Side::BUY) ? TradeType::BUY : TradeType::SELL;
    TransactionRepository::save(t);

    double total = calculate_portfolio_value(incoming.portfolio_id);
    PortfolioValueHistory pvh;
    pvh.portfolio_id = incoming.portfolio_id;
    pvh.value        = total;
    PortfolioValueHistoryRepository::save(pvh);

    std::cout << "[TRADE] " << to_string(t.type) << " " << qty
              << " " << incoming.symbol << " @ $" << price << std::endl;
}

double OrderMatchingService::calculate_portfolio_value(int64_t portfolio_id) {
    auto txns = TransactionRepository::find_by_portfolio_id(portfolio_id);

    std::unordered_map<std::string, int>    holdings;
    std::unordered_map<std::string, double> last_price;

    for (const auto& txn : txns) {
        int qty = (txn.type == TradeType::BUY) ? txn.quantity : -txn.quantity;
        holdings[txn.stock_symbol]   += qty;
        last_price[txn.stock_symbol]  = txn.price;
    }

    double total = 0.0;
    for (const auto& [symbol, shares] : holdings) {
        if (shares > 0) total += last_price.at(symbol) * shares;
    }
    return total;
}
