#include "stocks.h"
#include "../repositories.h"

void register_stock_routes(crow::SimpleApp& app, std::shared_ptr<MarketDataService> market) {

    // GET /api/stocks
    CROW_ROUTE(app, "/api/stocks").methods("GET"_method)
    ([market](const crow::request&) {
        auto stocks = StockRepository::find_all();

        std::vector<crow::json::wvalue> arr;
        arr.reserve(stocks.size());
        for (auto& s : stocks) {
            s.price = market->get_price(s.symbol);
            arr.push_back(s.to_json());
        }

        crow::response res(crow::json::wvalue(std::move(arr)).dump());
        res.set_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });

    // GET /api/stocks/<symbol>
    CROW_ROUTE(app, "/api/stocks/<string>").methods("GET"_method)
    ([market](const crow::request&, const std::string& symbol) {
        auto opt = StockRepository::find_by_symbol(symbol);
        if (!opt) {
            // Auto-create unknown symbols (matching Java behaviour)
            Stock s;
            s.symbol = symbol;
            s.name   = symbol;
            s.price  = market->get_price(symbol);
            StockRepository::save(s);
            crow::response res(s.to_json().dump());
            res.set_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
        opt->price = market->get_price(opt->symbol);
        crow::response res(opt->to_json().dump());
        res.set_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });
}
