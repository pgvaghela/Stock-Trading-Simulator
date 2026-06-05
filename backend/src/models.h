#pragma once
#include "crow.h"
#include <string>
#include <cstdint>

enum class TradeType { BUY, SELL };

inline std::string to_string(TradeType t) {
    return t == TradeType::BUY ? "BUY" : "SELL";
}
inline TradeType trade_type_from_string(const std::string& s) {
    return s == "BUY" ? TradeType::BUY : TradeType::SELL;
}

struct User {
    int64_t id = 0;
    std::string username;
    std::string password_hash;
    std::string created_at;

    crow::json::wvalue to_json() const {
        crow::json::wvalue j;
        j["id"] = id;
        j["username"] = username;
        j["createdAt"] = created_at;
        return j;
    }
};

struct Portfolio {
    int64_t id = 0;
    int64_t user_id = 0;

    crow::json::wvalue to_json() const {
        crow::json::wvalue j;
        j["id"] = id;
        j["user"]["id"] = user_id;
        return j;
    }
};

struct Stock {
    std::string symbol;
    std::string name;
    double price = 0.0;

    crow::json::wvalue to_json() const {
        crow::json::wvalue j;
        j["symbol"] = symbol;
        j["name"] = name;
        j["price"] = price;
        return j;
    }
};

struct Transaction {
    int64_t id = 0;
    int64_t portfolio_id = 0;
    std::string stock_symbol;
    int quantity = 0;
    double price = 0.0;
    TradeType type = TradeType::BUY;
    std::string timestamp;

    crow::json::wvalue to_json() const {
        crow::json::wvalue j;
        j["id"] = id;
        j["stockSymbol"] = stock_symbol;
        j["quantity"] = quantity;
        j["price"] = price;
        j["type"] = to_string(type);
        j["timestamp"] = timestamp;
        return j;
    }
};

struct PortfolioValueHistory {
    int64_t id = 0;
    int64_t portfolio_id = 0;
    std::string timestamp;
    double value = 0.0;

    crow::json::wvalue to_json() const {
        crow::json::wvalue j;
        j["id"] = id;
        j["timestamp"] = timestamp;
        j["value"] = value;
        return j;
    }
};

struct MarketQuote {
    std::string symbol;
    double price = 0.0;
    std::string timestamp;

    std::string to_json_string() const {
        crow::json::wvalue j;
        j["symbol"] = symbol;
        j["price"] = price;
        j["timestamp"] = timestamp;
        return j.dump();
    }
};

struct OrderRequest {
    int64_t portfolio_id = 0;
    std::string symbol;
    int quantity = 0;
    std::string side; // "BUY" or "SELL"
    double price = 0.0;
};
