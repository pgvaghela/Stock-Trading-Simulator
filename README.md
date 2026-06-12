# Stock Trading Simulator

Real-time stock trading simulator with a **C++17** backend (Crow, SQLite, AWS CloudWatch) and a React + Tailwind CSS frontend. The original Java/Spring Boot implementation is preserved in `java-backend/` for reference.

## Features

- **Real-time trading** — buy and sell with live Alpha Vantage prices, mock fallback on rate-limit
- **Order-book engine** — priority-queue BUY max-heap / SELL min-heap matched under `std::mutex`
- **Live price push** — background `std::thread` polls tickers every 60 s and broadcasts via WebSocket
- **Portfolio tracking** — value-history snapshots on every trade, performance chart in the dashboard
- **AWS CloudWatch** — publishes `OrdersPerMinute` metrics via the C++ SDK (`StockSimulator` namespace)

## Tech Stack

| Layer | Technology |
|---|---|
| HTTP server + WebSocket | [Crow v1.2](https://github.com/CrowCpp/Crow) |
| Database | SQLite 3 |
| Market data client | [cpp-httplib](https://github.com/yhirose/cpp-httplib) + OpenSSL |
| Metrics | AWS CloudWatch (C++ SDK) |
| Build | CMake 3.16+, Docker (4-stage) |
| Frontend | React 18, Tailwind CSS, Recharts |

## Project Structure

```
Stock-Trading-Simulator/
├── backend/                          # C++17 backend (current)
│   ├── CMakeLists.txt
│   ├── config.env.template
│   └── src/
│       ├── main.cpp                  # Crow app, WebSocket hub, static file server
│       ├── models.h                  # Data structs: User, Portfolio, Stock, Transaction…
│       ├── database.h/.cpp           # SQLite wrapper + schema init
│       ├── repositories.h/.cpp       # Prepared-statement CRUD layer
│       ├── services/
│       │   ├── order_matching.h/.cpp # Priority-queue order book
│       │   ├── market_data.h/.cpp    # Alpha Vantage polling + WebSocket broadcast
│       │   └── metrics.h/.cpp        # AWS CloudWatch PutMetricData
│       └── routes/
│           ├── users.h/.cpp          # POST /api/users
│           ├── portfolios.h/.cpp     # GET/POST /api/portfolios
│           ├── stocks.h/.cpp         # GET /api/stocks
│           ├── orders.h/.cpp         # POST /api/orders
│           └── value_history.h/.cpp  # GET /api/portfolios/{id}/value-history
├── java-backend/                     # Original Spring Boot / Java 17 implementation
│   ├── pom.xml
│   └── src/
│       └── main/java/com/example/demo/
│           ├── controller/           # REST controllers
│           ├── entity/               # JPA entities
│           ├── repository/           # Spring Data repositories
│           ├── service/              # Business logic
│           └── websocket/            # STOMP WebSocket config
├── Dockerfile                        # 4-stage: React → AWS SDK → C++ binary → runtime
└── README.md
```

## Quick Start

### Prerequisites

- CMake ≥ 3.16
- GCC or Clang with C++17 support
- `libsqlite3-dev`, `libssl-dev` (Ubuntu) / `sqlite3`, `openssl` (Homebrew)
- Node.js ≥ 16 (to rebuild the React frontend)
- Git (CMake FetchContent pulls Crow and cpp-httplib automatically)

### 1. Build the C++ backend

```bash
cd backend
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### 2. Build the React frontend

```bash
cd java-backend/src/main/resources/static/frontend
npm install
npm run build
```

### 3. Configure and run

```bash
cp backend/config.env.template config.env
# Edit config.env — set MARKET_API_KEY to your Alpha Vantage key
source config.env
export STATIC_DIR="$(pwd)/java-backend/src/main/resources/static/frontend/build"
./backend/build/stock_simulator
```

Open http://localhost:8080

### Docker (recommended)

```bash
docker build -t stock-trading-simulator .
docker run -p 8080:8080 -e MARKET_API_KEY=your_key stock-trading-simulator
```

## API

| Method | Path | Description |
|--------|------|-------------|
| `POST` | `/api/users` | Create a user |
| `POST` | `/api/portfolios` | Create or fetch portfolio for a user |
| `GET` | `/api/portfolios/{id}` | Get portfolio by ID |
| `GET` | `/api/portfolios/{id}/transactions` | List all transactions |
| `GET` | `/api/portfolios/{id}/value-history` | Portfolio value snapshots |
| `POST` | `/api/orders` | Place a BUY or SELL order |
| `GET` | `/api/stocks` | All tracked stocks with live prices |
| `GET` | `/api/stocks/{symbol}` | Single stock with live price |
| `WS` | `/ws` | Receives `{symbol, price, timestamp}` on every price refresh |

## Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `DB_PATH` | `stock_simulator.db` | SQLite file path |
| `MARKET_API_KEY` | `demo` | Alpha Vantage API key |
| `MARKET_API_URL` | `https://www.alphavantage.co` | Alpha Vantage base URL |
| `STATIC_DIR` | *(see config.env.template)* | Path to built React `index.html` |
| `PORT` | `8080` | HTTP listen port |

## License

MIT
