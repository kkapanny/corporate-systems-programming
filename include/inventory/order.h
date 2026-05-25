#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace inventory {

struct OrderItem {
    std::int64_t product_id = 0;
    double quantity = 0.0;
};

struct ReplenishmentOrder {
    std::int64_t id = 0;
    std::chrono::system_clock::time_point created_at{};
    std::string status;
    std::vector<OrderItem> items;
};

}
