#include "portfolios.h"
#include "../repositories.h"

static crow::response cors_ok(int status = 204) {
    crow::response res(status);
    res.add_header("Access-Control-Allow-Origin",  "*");
    res.add_header("Access-Control-Allow-Headers", "Content-Type");
    res.add_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    return res;
}

void register_portfolio_routes(crow::SimpleApp& app) {

    // POST /api/portfolios  — create or return existing portfolio for a user
    CROW_ROUTE(app, "/api/portfolios").methods("POST"_method, "OPTIONS"_method)
    ([](const crow::request& req) {
        if (req.method == crow::HTTPMethod::Options) return cors_ok();

        auto body = crow::json::load(req.body);
        if (!body || !body.has("user")) {
            crow::response res(400, "Missing user");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }

        int64_t user_id = body["user"]["id"].i();

        auto existing = PortfolioRepository::find_by_user_id(user_id);
        if (!existing.empty()) {
            crow::response res(existing[0].to_json().dump());
            res.set_header("Content-Type", "application/json");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }

        Portfolio p;
        p.user_id = user_id;
        Portfolio saved = PortfolioRepository::save(p);

        crow::response res(saved.to_json().dump());
        res.set_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });

    // GET /api/portfolios/<id>
    CROW_ROUTE(app, "/api/portfolios/<int>").methods("GET"_method, "OPTIONS"_method)
    ([](const crow::request& req, int id) {
        if (req.method == crow::HTTPMethod::Options) return cors_ok();

        auto p = PortfolioRepository::find_by_id(static_cast<int64_t>(id));
        if (!p) {
            crow::response res(404, "Not found");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
        crow::response res(p->to_json().dump());
        res.set_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });

    // GET /api/portfolios/<id>/transactions
    CROW_ROUTE(app, "/api/portfolios/<int>/transactions").methods("GET"_method, "OPTIONS"_method)
    ([](const crow::request& req, int id) {
        if (req.method == crow::HTTPMethod::Options) return cors_ok();

        if (!PortfolioRepository::exists_by_id(static_cast<int64_t>(id))) {
            crow::response res(404, "Portfolio not found");
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }

        auto txns = TransactionRepository::find_by_portfolio_id(static_cast<int64_t>(id));
        std::vector<crow::json::wvalue> arr;
        arr.reserve(txns.size());
        for (const auto& t : txns) arr.push_back(t.to_json());

        crow::response res(crow::json::wvalue(std::move(arr)).dump());
        res.set_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });
}
