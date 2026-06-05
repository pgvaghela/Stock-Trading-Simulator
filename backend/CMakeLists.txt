cmake_minimum_required(VERSION 3.16)
project(StockTradingSimulator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Crow — HTTP server + WebSocket
FetchContent_Declare(Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.2.0
)
FetchContent_MakeAvailable(Crow)

# cpp-httplib — HTTPS client for Alpha Vantage
FetchContent_Declare(httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.14.3
)
FetchContent_MakeAvailable(httplib)

find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# Optional AWS SDK — enables real CloudWatch metrics publishing.
# Install: build from source (see Dockerfile) or cmake --install the pre-built SDK.
find_package(AWSSDK QUIET COMPONENTS cloudwatch)
if(AWSSDK_FOUND)
    message(STATUS "AWS SDK found — CloudWatch metrics ENABLED")
else()
    message(STATUS "AWS SDK not found — metrics will log to stdout (CloudWatch calls stubbed)")
    message(STATUS "  To enable: build aws-sdk-cpp with -DBUILD_ONLY=cloudwatch and install it")
endif()

set(SOURCES
    src/main.cpp
    src/database.cpp
    src/repositories.cpp
    src/services/order_matching.cpp
    src/services/market_data.cpp
    src/services/metrics.cpp
    src/routes/orders.cpp
    src/routes/portfolios.cpp
    src/routes/stocks.cpp
    src/routes/users.cpp
    src/routes/value_history.cpp
)

add_executable(stock_simulator ${SOURCES})

target_include_directories(stock_simulator PRIVATE src/)

# CPPHTTPLIB_OPENSSL_SUPPORT enables HTTPS in cpp-httplib
target_compile_definitions(stock_simulator PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)

if(AWSSDK_FOUND)
    target_compile_definitions(stock_simulator PRIVATE HAVE_AWS_SDK)
    target_include_directories(stock_simulator PRIVATE ${AWSSDK_INCLUDE_DIRS})
endif()

target_link_libraries(stock_simulator
    PRIVATE
        Crow::Crow
        httplib::httplib
        SQLite::SQLite3
        OpenSSL::SSL
        OpenSSL::Crypto
        Threads::Threads
        $<$<BOOL:${AWSSDK_FOUND}>:${AWSSDK_LINK_LIBRARIES}>
)
