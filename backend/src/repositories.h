#pragma once
#include "models.h"
#include <vector>
#include <optional>

struct UserRepository {
    static std::optional<User>          find_by_id(int64_t id);
    static std::optional<User>          find_by_username(const std::string& username);
    static User                         save(User user);
};

struct PortfolioRepository {
    static std::optional<Portfolio>     find_by_id(int64_t id);
    static std::vector<Portfolio>       find_by_user_id(int64_t user_id);
    static bool                         exists_by_id(int64_t id);
    static Portfolio                    save(Portfolio p);
};

struct StockRepository {
    static std::vector<Stock>           find_all();
    static std::optional<Stock>         find_by_symbol(const std::string& symbol);
    static Stock                        save(Stock s);
    static int64_t                      count();
};

struct TransactionRepository {
    static std::vector<Transaction>     find_by_portfolio_id(int64_t portfolio_id);
    static Transaction                  save(Transaction t);
};

struct PortfolioValueHistoryRepository {
    static std::vector<PortfolioValueHistory> find_by_portfolio_id(int64_t portfolio_id);
    static PortfolioValueHistory              save(PortfolioValueHistory pvh);
};
