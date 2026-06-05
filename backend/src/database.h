#pragma once
#include <sqlite3.h>
#include <string>
#include <functional>
#include <stdexcept>
#include <memory>
#include <mutex>

class Database {
public:
    explicit Database(const std::string& path);
    ~Database();

    // Non-copyable
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    // Execute DDL / DML with no results
    void execute(const std::string& sql);

    using RowCallback = std::function<void(sqlite3_stmt*)>;

    // Execute a parameterized query; bind_fn sets the bound parameters before step
    void query(const std::string& sql, RowCallback row_cb,
               std::function<void(sqlite3_stmt*)> bind_fn = nullptr);

    int64_t last_insert_rowid();

    // Global write lock — callers should hold this around any multi-step write
    std::mutex& write_mutex() { return write_mtx_; }

private:
    sqlite3* db_ = nullptr;
    std::mutex write_mtx_;
};

// Process-wide singleton
Database& get_db();
void init_db(const std::string& path = "stock_simulator.db");
void seed_db();
