#include "database.h"
#include <iostream>

Database::Database(const std::string& path) {
    int rc = sqlite3_open(path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::string err = sqlite3_errmsg(db_);
        sqlite3_close(db_);
        throw std::runtime_error("Cannot open database: " + err);
    }
    execute("PRAGMA journal_mode=WAL");
    execute("PRAGMA foreign_keys=ON");
}

Database::~Database() {
    if (db_) sqlite3_close(db_);
}

void Database::execute(const std::string& sql) {
    char* errmsg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
        std::string err = errmsg ? errmsg : "unknown error";
        sqlite3_free(errmsg);
        throw std::runtime_error("SQL error: " + err);
    }
}

void Database::query(const std::string& sql, RowCallback row_cb,
                     std::function<void(sqlite3_stmt*)> bind_fn) {
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare: " + std::string(sqlite3_errmsg(db_)));
    }
    if (bind_fn) bind_fn(stmt);
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        row_cb(stmt);
    }
    sqlite3_finalize(stmt);
}

int64_t Database::last_insert_rowid() {
    return sqlite3_last_insert_rowid(db_);
}

// ---------- singleton ----------

static std::unique_ptr<Database> g_db;

Database& get_db() {
    if (!g_db) throw std::runtime_error("Database not initialized");
    return *g_db;
}

void init_db(const std::string& path) {
    g_db = std::make_unique<Database>(path);

    get_db().execute(R"(
        CREATE TABLE IF NOT EXISTS users (
            id            INTEGER PRIMARY KEY AUTOINCREMENT,
            username      TEXT    UNIQUE NOT NULL,
            password_hash TEXT    NOT NULL,
            created_at    DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )");

    get_db().execute(R"(
        CREATE TABLE IF NOT EXISTS portfolios (
            id      INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            FOREIGN KEY (user_id) REFERENCES users(id)
        )
    )");

    get_db().execute(R"(
        CREATE TABLE IF NOT EXISTS stocks (
            symbol TEXT PRIMARY KEY,
            name   TEXT NOT NULL,
            price  REAL DEFAULT 0.0
        )
    )");

    get_db().execute(R"(
        CREATE TABLE IF NOT EXISTS transactions (
            id           INTEGER PRIMARY KEY AUTOINCREMENT,
            portfolio_id INTEGER NOT NULL,
            stock_symbol TEXT    NOT NULL,
            quantity     INTEGER NOT NULL,
            price        REAL    NOT NULL,
            type         TEXT    NOT NULL CHECK(type IN ('BUY','SELL')),
            timestamp    DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (portfolio_id) REFERENCES portfolios(id)
        )
    )");

    get_db().execute(R"(
        CREATE TABLE IF NOT EXISTS portfolio_value_history (
            id           INTEGER PRIMARY KEY AUTOINCREMENT,
            portfolio_id INTEGER NOT NULL,
            timestamp    DATETIME DEFAULT CURRENT_TIMESTAMP,
            value        REAL    NOT NULL,
            FOREIGN KEY (portfolio_id) REFERENCES portfolios(id)
        )
    )");
}

void seed_db() {
    int count = 0;
    get_db().query("SELECT COUNT(*) FROM stocks",
        [&count](sqlite3_stmt* s) { count = sqlite3_column_int(s, 0); });

    if (count == 0) {
        get_db().execute(R"(
            INSERT INTO stocks (symbol, name) VALUES
                ('AAPL','Apple Inc.'),
                ('GOOG','Alphabet Inc.'),
                ('MSFT','Microsoft Corp.'),
                ('TSLA','Tesla Inc.'),
                ('AMZN','Amazon.com Inc.')
        )");
        std::cout << "Sample stocks initialized!" << std::endl;
    }
}
