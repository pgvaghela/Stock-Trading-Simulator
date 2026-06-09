#include "users.h"
#include "../repositories.h"
#include <chrono>

void register_user_routes(crow::SimpleApp& app) {

    // POST /api/users
    CROW_ROUTE(app, "/api/users").methods("POST"_method, "OPTIONS"_method)
    ([](const crow::request& req) {
        if (req.method == crow::HTTPMethod::Options) {
            crow::response res(204);
            res.add_header("Access-Control-Allow-Origin",  "*");
            res.add_header("Access-Control-Allow-Headers", "Content-Type");
            res.add_header("Access-Control-Allow-Methods", "POST, OPTIONS");
            return res;
        }

        User user;
        if (!req.body.empty()) {
            auto body = crow::json::load(req.body);
            if (body && body.has("username"))
                user.username = body["username"].s();
        }

        if (user.username.empty()) {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            user.username = "user" + std::to_string(ms);
        }
        if (user.password_hash.empty()) user.password_hash = "demo";

        User saved = UserRepository::save(user);

        crow::response res(saved.to_json().dump());
        res.set_header("Content-Type", "application/json");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });
}
