# Stock Trading Simulator

A real-time stock trading simulator. The backend is written in **C++17** using [Crow](https://github.com/CrowCpp/Crow) for HTTP/WebSocket, SQLite for persistence, and [cpp-httplib](https://github.com/yhirose/cpp-httplib) for Alpha Vantage market data. The frontend is React + Tailwind CSS.

## Features

- **Real-time stock trading** — buy and sell with live Alpha Vantage prices (mock fallback when rate-limited)
- **Portfolio management** — track holdings, position value, and transaction history
- **Live price push** — WebSocket endpoint broadcasts price updates to the React dashboard every 60 s
- **Portfolio performance chart** — value-history snapshots recorded on every trade
- **Order-book matching engine** — priority-queue BUY max-heap / SELL min-heap matching orders by price

## Tech Stack

| Layer | Technology |
|---|---|
| HTTP server + WebSocket | [Crow v1.2](https://github.com/CrowCpp/Crow) |
| Database | SQLite 3 |
| Market data HTTP client | [cpp-httplib](https://github.com/yhirose/cpp-httplib) + OpenSSL |
| Build system | CMake 3.16+ |
| Frontend | React 18, Tailwind CSS, Recharts |
| Container | Docker (multi-stage) |

## Project Structure

```
Stock-Trading-Simulator/
├── backend/                     # C++ backend
│   ├── CMakeLists.txt
│   ├── config.env.template
│   └── src/
│       ├── main.cpp             # Entry point, Crow app, WebSocket hub
│       ├── models.h             # Data structs (User, Portfolio, Stock, Transaction…)
│       ├── database.h/.cpp      # SQLite wrapper + schema init
│       ├── repositories.h/.cpp  # CRUD data-access layer
│       ├── services/
│       │   ├── order_matching.h/.cpp   # Priority-queue order book
│       │   ├── market_data.h/.cpp      # Alpha Vantage polling + WS broadcast
│       │   └── metrics.h/.cpp         # Operational metrics logging
│       └── routes/
│           ├── users.h/.cpp           # POST /api/users
│           ├── portfolios.h/.cpp      # GET/POST /api/portfolios
│           ├── stocks.h/.cpp          # GET /api/stocks
│           ├── orders.h/.cpp          # POST /api/orders
│           └── value_history.h/.cpp   # GET /api/portfolios/{id}/value-history
├── demo/                        # Original Java/Spring Boot source (reference)
├── Dockerfile                   # Multi-stage: React build → C++ build → runtime
└── README.md
```

## Quick Start

### Prerequisites

- CMake ≥ 3.16
- GCC or Clang with C++17 support
- SQLite3 dev headers (`libsqlite3-dev` on Ubuntu)
- OpenSSL dev headers (`libssl-dev` on Ubuntu)
- Node.js ≥ 16 (to rebuild the React frontend)
- Git (CMake FetchContent clones Crow + cpp-httplib automatically)

### 1. Build the C++ backend

```bash
cd backend
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### 2. Build the React frontend

```bash
cd demo/src/main/resources/static/frontend
npm install
npm run build
```

### 3. Configure and run

```bash
cp backend/config.env.template config.env
# Edit config.env — set MARKET_API_KEY
source config.env
export STATIC_DIR="$(pwd)/demo/src/main/resources/static/frontend/build"
./backend/build/stock_simulator
```

Open http://localhost:8080

### Docker (recommended)

```bash
# Set your Alpha Vantage key
docker build -t stock-trading-simulator .
docker run -p 8080:8080 -e MARKET_API_KEY=your_key stock-trading-simulator
```

## API Endpoints

| Method | Path | Description |
|--------|------|-------------|
| POST | `/api/users` | Create a user |
| POST | `/api/portfolios` | Create/fetch portfolio for a user |
| GET | `/api/portfolios/{id}` | Get portfolio by ID |
| GET | `/api/portfolios/{id}/transactions` | List all transactions |
| GET | `/api/portfolios/{id}/value-history` | Portfolio value over time |
| POST | `/api/orders` | Place a BUY or SELL order |
| GET | `/api/stocks` | List all tracked stocks with live prices |
| GET | `/api/stocks/{symbol}` | Get a single stock with live price |
| WS | `/ws` | WebSocket — receives `{symbol, price, timestamp}` JSON on every price refresh |

## Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `DB_PATH` | `stock_simulator.db` | SQLite file path |
| `MARKET_API_KEY` | `demo` | Alpha Vantage API key |
| `MARKET_API_URL` | `https://www.alphavantage.co` | Alpha Vantage base URL |
| `STATIC_DIR` | *(relative path to build/)* | Directory containing `index.html` |
| `PORT` | `8080` | HTTP listen port |

## License

MIT
