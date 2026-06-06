#include "repositories.h"
#include "database.h"
#include <sqlite3.h>

// ---------- helpers ----------

static std::string col_text(sqlite3_stmt* s, int col) {
    const char* t = reinterpret_cast<const char*>(sqlite3_column_text(s, col));
    return t ? t : "";
}

// ---------- UserRepository ----------

std::optional<User> UserRepository::find_by_id(int64_t id) {
    std::optional<User> result;
    get_db().query(
        "SELECT id,username,password_hash,created_at FROM users WHERE id=?",
        [&result](sqlite3_stmt* s) {
            User u;
            u.id            = sqlite3_column_int64(s, 0);
            u.username      = col_text(s, 1);
            u.password_hash = col_text(s, 2);
            u.created_at    = col_text(s, 3);
            result = u;
        },
        [id](sqlite3_stmt* s) { sqlite3_bind_int64(s, 1, id); }
    );
    return result;
}

std::optional<User> UserRepository::find_by_username(const std::string& username) {
    std::optional<User> result;
    get_db().query(
        "SELECT id,username,password_hash,created_at FROM users WHERE username=?",
        [&result](sqlite3_stmt* s) {
            User u;
            u.id            = sqlite3_column_int64(s, 0);
            u.username      = col_text(s, 1);
            u.password_hash = col_text(s, 2);
            u.created_at    = col_text(s, 3);
            result = u;
        },
        [&username](sqlite3_stmt* s) { sqlite3_bind_text(s, 1, username.c_str(), -1, SQLITE_TRANSIENT); }
    );
    return result;
}

User UserRepository::save(User user) {
    std::lock_guard<std::mutex> lock(get_db().write_mutex());
    get_db().query(
        "INSERT INTO users(username,password_hash) VALUES(?,?) "
        "ON CONFLICT(username) DO UPDATE SET password_hash=excluded.password_hash "
        "RETURNING id,username,password_hash,created_at",
        [&user](sqlite3_stmt* s) {
            user.id            = sqlite3_column_int64(s, 0);
            user.username      = col_text(s, 1);
            user.password_hash = col_text(s, 2);
            user.created_at    = col_text(s, 3);
        },
        [&user](sqlite3_stmt* s) {
            sqlite3_bind_text(s, 1, user.username.c_str(),      -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(s, 2, user.password_hash.c_str(), -1, SQLITE_TRANSIENT);
        }
    );
    return user;
}

// ---------- PortfolioRepository ----------

std::optional<Portfolio> PortfolioRepository::find_by_id(int64_t id) {
    std::optional<Portfolio> result;
    get_db().query(
        "SELECT id,user_id FROM portfolios WHERE id=?",
        [&result](sqlite3_stmt* s) {
            Portfolio p;
            p.id      = sqlite3_column_int64(s, 0);
            p.user_id = sqlite3_column_int64(s, 1);
            result = p;
        },
        [id](sqlite3_stmt* s) { sqlite3_bind_int64(s, 1, id); }
    );
    return result;
}

std::vector<Portfolio> PortfolioRepository::find_by_user_id(int64_t user_id) {
    std::vector<Portfolio> result;
    get_db().query(
        "SELECT id,user_id FROM portfolios WHERE user_id=?",
        [&result](sqlite3_stmt* s) {
            Portfolio p;
            p.id      = sqlite3_column_int64(s, 0);
            p.user_id = sqlite3_column_int64(s, 1);
            result.push_back(p);
        },
        [user_id](sqlite3_stmt* s) { sqlite3_bind_int64(s, 1, user_id); }
    );
    return result;
}

bool PortfolioRepository::exists_by_id(int64_t id) {
    int count = 0;
    get_db().query(
        "SELECT COUNT(*) FROM portfolios WHERE id=?",
        [&count](sqlite3_stmt* s) { count = sqlite3_column_int(s, 0); },
        [id](sqlite3_stmt* s) { sqlite3_bind_int64(s, 1, id); }
    );
    return count > 0;
}

Portfolio PortfolioRepository::save(Portfolio p) {
    std::lock_guard<std::mutex> lock(get_db().write_mutex());
    get_db().query(
        "INSERT INTO portfolios(user_id) VALUES(?) RETURNING id,user_id",
        [&p](sqlite3_stmt* s) {
            p.id      = sqlite3_column_int64(s, 0);
            p.user_id = sqlite3_column_int64(s, 1);
        },
        [&p](sqlite3_stmt* s) { sqlite3_bind_int64(s, 1, p.user_id); }
    );
    return p;
}

// ---------- StockRepository ----------

std::vector<Stock> StockRepository::find_all() {
    std::vector<Stock> result;
    get_db().query(
        "SELECT symbol,name,price FROM stocks",
        [&result](sqlite3_stmt* s) {
            Stock st;
            st.symbol = col_text(s, 0);
            st.name   = col_text(s, 1);
            st.price  = sqlite3_column_double(s, 2);
            result.push_back(st);
        }
    );
    return result;
}

std::optional<Stock> StockRepository::find_by_symbol(const std::string& symbol) {
    std::optional<Stock> result;
    get_db().query(
        "SELECT symbol,name,price FROM stocks WHERE symbol=?",
        [&result](sqlite3_stmt* s) {
            Stock st;
            st.symbol = col_text(s, 0);
            st.name   = col_text(s, 1);
            st.price  = sqlite3_column_double(s, 2);
            result = st;
        },
        [&symbol](sqlite3_stmt* s) {
            sqlite3_bind_text(s, 1, symbol.c_str(), -1, SQLITE_TRANSIENT);
        }
    );
    return result;
}

Stock StockRepository::save(Stock st) {
    std::lock_guard<std::mutex> lock(get_db().write_mutex());
    get_db().query(
        "INSERT INTO stocks(symbol,name,price) VALUES(?,?,?) "
        "ON CONFLICT(symbol) DO UPDATE SET name=excluded.name, price=excluded.price "
        "RETURNING symbol,name,price",
        [&st](sqlite3_stmt* s) {
            st.symbol = col_text(s, 0);
            st.name   = col_text(s, 1);
            st.price  = sqlite3_column_double(s, 2);
        },
        [&st](sqlite3_stmt* s) {
            sqlite3_bind_text(  s, 1, st.symbol.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(  s, 2, st.name.c_str(),   -1, SQLITE_TRANSIENT);
            sqlite3_bind_double(s, 3, st.price);
        }
    );
    return st;
}

int64_t StockRepository::count() {
    int64_t cnt = 0;
    get_db().query("SELECT COUNT(*) FROM stocks",
        [&cnt](sqlite3_stmt* s) { cnt = sqlite3_column_int64(s, 0); });
    return cnt;
}

// ---------- TransactionRepository ----------

std::vector<Transaction> TransactionRepository::find_by_portfolio_id(int64_t portfolio_id) {
    std::vector<Transaction> result;
    get_db().query(
        "SELECT id,portfolio_id,stock_symbol,quantity,price,type,timestamp "
        "FROM transactions WHERE portfolio_id=? ORDER BY timestamp ASC",
        [&result](sqlite3_stmt* s) {
            Transaction t;
            t.id           = sqlite3_column_int64(s, 0);
            t.portfolio_id = sqlite3_column_int64(s, 1);
            t.stock_symbol = col_text(s, 2);
            t.quantity     = sqlite3_column_int(s, 3);
            t.price        = sqlite3_column_double(s, 4);
            t.type         = trade_type_from_string(col_text(s, 5));
            t.timestamp    = col_text(s, 6);
            result.push_back(t);
        },
        [portfolio_id](sqlite3_stmt* s) { sqlite3_bind_int64(s, 1, portfolio_id); }
    );
    return result;
}

Transaction TransactionRepository::save(Transaction t) {
    std::lock_guard<std::mutex> lock(get_db().write_mutex());
    std::string type_str = to_string(t.type);
    get_db().query(
        "INSERT INTO transactions(portfolio_id,stock_symbol,quantity,price,type) "
        "VALUES(?,?,?,?,?) RETURNING id,timestamp",
        [&t](sqlite3_stmt* s) {
            t.id        = sqlite3_column_int64(s, 0);
            t.timestamp = col_text(s, 1);
        },
        [&t, &type_str](sqlite3_stmt* s) {
            sqlite3_bind_int64(s, 1, t.portfolio_id);
            sqlite3_bind_text( s, 2, t.stock_symbol.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(  s, 3, t.quantity);
            sqlite3_bind_double(s, 4, t.price);
            sqlite3_bind_text( s, 5, type_str.c_str(), -1, SQLITE_TRANSIENT);
        }
    );
    return t;
}

// ---------- PortfolioValueHistoryRepository ----------

std::vector<PortfolioValueHistory> PortfolioValueHistoryRepository::find_by_portfolio_id(
    int64_t portfolio_id) {
    std::vector<PortfolioValueHistory> result;
    get_db().query(
        "SELECT id,portfolio_id,timestamp,value FROM portfolio_value_history "
        "WHERE portfolio_id=? ORDER BY timestamp ASC",
        [&result](sqlite3_stmt* s) {
            PortfolioValueHistory pvh;
            pvh.id           = sqlite3_column_int64(s, 0);
            pvh.portfolio_id = sqlite3_column_int64(s, 1);
            pvh.timestamp    = col_text(s, 2);
            pvh.value        = sqlite3_column_double(s, 3);
            result.push_back(pvh);
        },
        [portfolio_id](sqlite3_stmt* s) { sqlite3_bind_int64(s, 1, portfolio_id); }
    );
    return result;
}

PortfolioValueHistory PortfolioValueHistoryRepository::save(PortfolioValueHistory pvh) {
    std::lock_guard<std::mutex> lock(get_db().write_mutex());
    get_db().query(
        "INSERT INTO portfolio_value_history(portfolio_id,value) VALUES(?,?) "
        "RETURNING id,timestamp",
        [&pvh](sqlite3_stmt* s) {
            pvh.id        = sqlite3_column_int64(s, 0);
            pvh.timestamp = col_text(s, 1);
        },
        [&pvh](sqlite3_stmt* s) {
            sqlite3_bind_int64( s, 1, pvh.portfolio_id);
            sqlite3_bind_double(s, 2, pvh.value);
        }
    );
    return pvh;
}
