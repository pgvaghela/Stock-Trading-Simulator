#pragma once
#include "crow.h"
#include <memory>
#include "../services/market_data.h"

void register_value_history_routes(crow::SimpleApp& app,
                                   std::shared_ptr<MarketDataService> market);
