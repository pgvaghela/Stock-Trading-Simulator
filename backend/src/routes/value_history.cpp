#include "value_history.h"
#include "../repositories.h"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <iostream>

static std::string now_iso() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&t));
    return buf;
}

static std::string offset_iso(int seconds_ago) {
    auto now = std::chrono::system_clock::now()
             - std::chrono::seconds(seconds_ago);
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&t));
    return buf;
}

static double calc_portfolio_value(int64_t portfolio_id,
                                    std::shared_ptr<MarketDataService> market) {
    auto txns = TransactionRepository::find_by_portfolio_id(portfolio_id);

    std::unordered_map<std::string, int> holdings;
    for (const auto& t : txns) {
        int qty = (t.type == TradeType::BUY) ? t.quantity : -t.quantity;
        holdings[t.stock_symbol] += qty;
    }

    double total = 0.0;
    for (const auto& [sym, qty] : holdings) {
        if (qty > 0) total += market->get_price(sym) * qty;
    }
    return total;
}

void register_value_history_routes(crow::SimpleApp& app,
                                    std::shared_ptr<MarketDataService> market) {

    // GET /api/portfolios/<id>/value-history
    CROW_ROUTE(app, "/api/portfolios/<int>/value-history").methods("GET"_method)
    ([market](const crow::request&, int id) {
        int64_t portfolio_id = static_cast<int64_t>(id);

        // Try to return real persisted snapshots first
        auto history = PortfolioValueHistoryRepository::find_by_portfolio_id(portfolio_id);

        std::vector<crow::json::wvalue> arr;

        if (!history.empty()) {
            arr.reserve(history.size());
            for (const auto& h : history) arr.push_back(h.to_json());
        } else {
            // Synthesise chart data from current portfolio value (mirrors Java's
            // createRealisticChartData) so the frontend always has something to draw.
            double current = 0.0;
            try {
                current = calc_portfolio_value(portfolio_id, market);
            } catch (const std::exception& e) {
                std::cerr << "[ValueHistory] " << e.what() << std::endl;
            }

            if (current <= 0.0) {
                // Empty portfolio — flat line at $0
                for (int i = 5; i >= 0; --i) {
                    PortfolioValueHistory h;
                    h.timestamp = offset_iso(i * 60);
                    h.value     = 0.0;
                    arr.push_back(h.to_json());
                }
            } else {
                double step = current / 6.0;
                for (int i = 5; i >= 0; --i) {
                    PortfolioValueHistory h;
                    h.timestamp = offset_iso(i * 60);
                    h.value     = step * static_cast<double>(6 - i);
                    arr.push_back(h.to_json());
                }
            }
        }

        crow::response res(crow::json::wvalue(std::move(arr)).dump());
        res.set_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });
}
