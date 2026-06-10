#include "orders.h"
#include "../repositories.h"
#include <iostream>
#include <unordered_map>

// portfolioId may arrive as a JSON number OR as a string (when React restores
// the value from localStorage, it re-hydrates it as a string).
static int64_t parse_portfolio_id(const crow::json::rvalue& body) {
    const auto& v = body["portfolioId"];
    if (v.t() == crow::json::type::Number) return v.i();
    try { return std::stoll(std::string(v.s())); }
    catch (...) { return 0; }
}

static double current_price(const std::string& symbol,
                              std::shared_ptr<MarketDataService> market) {
    double p = market->get_price(symbol);
    return p > 0.0 ? p : 100.0;
}

static double calculate_portfolio_value(int64_t portfolio_id,
                                         std::shared_ptr<MarketDataService> market) {
    auto txns = TransactionRepository::find_by_portfolio_id(portfolio_id);
    std::unordered_map<std::string, int> holdings;
    for (const auto& t : txns) {
        int qty = (t.type == TradeType::BUY) ? t.quantity : -t.quantity;
        holdings[t.stock_symbol] += qty;
    }
    double total = 0.0;
    for (const auto& [sym, qty] : holdings)
        if (qty > 0) total += current_price(sym, market) * qty;
    return total;
}

void register_order_routes(crow::SimpleApp& app, std::shared_ptr<MarketDataService> market) {

    // POST /api/orders
    CROW_ROUTE(app, "/api/orders").methods("POST"_method, "OPTIONS"_method)
    ([market](const crow::request& req) {
        if (req.method == crow::HTTPMethod::Options) {
            crow::response res(204);
            res.add_header("Access-Control-Allow-Origin",  "*");
            res.add_header("Access-Control-Allow-Headers", "Content-Type");
            res.add_header("Access-Control-Allow-Methods", "POST, OPTIONS");
            return res;
        }

        auto body = crow::json::load(req.body);
        if (!body) {
            crow::response res(400, "Invalid JSON");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }

        OrderRequest order;
        order.portfolio_id = parse_portfolio_id(body);
        order.symbol       = body["symbol"].s();
        order.quantity     = static_cast<int>(body["quantity"].i());
        order.side         = body["side"].s();
        order.price        = body["price"].d();

        std::cout << "[ORDER] " << order.side << " " << order.quantity
                  << " " << order.symbol << " @ " << order.price
                  << " (portfolio=" << order.portfolio_id << ")" << std::endl;

        if (!PortfolioRepository::exists_by_id(order.portfolio_id)) {
            crow::response res(404, "Portfolio not found");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }

        // Ensure stock row exists
        if (!StockRepository::find_by_symbol(order.symbol)) {
            Stock s;
            s.symbol = order.symbol;
            s.name   = order.symbol;
            StockRepository::save(s);
        }

        // Persist transaction
        Transaction t;
        t.portfolio_id = order.portfolio_id;
        t.stock_symbol = order.symbol;
        t.quantity     = order.quantity;
        t.price        = order.price;
        t.type         = (order.side == "BUY") ? TradeType::BUY : TradeType::SELL;
        Transaction saved = TransactionRepository::save(t);

        // Record portfolio value snapshot
        try {
            double total = calculate_portfolio_value(order.portfolio_id, market);
            PortfolioValueHistory pvh;
            pvh.portfolio_id = order.portfolio_id;
            pvh.value        = total;
            PortfolioValueHistoryRepository::save(pvh);
        } catch (const std::exception& e) {
            std::cerr << "[ORDER] Failed to save value history: " << e.what() << std::endl;
        }

        crow::response res(saved.to_json().dump());
        res.set_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });
}
