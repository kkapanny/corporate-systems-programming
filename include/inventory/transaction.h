#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace inventory {

enum class TransactionType { Income, Expense };

struct StockTransaction {
    std::int64_t id = 0;
    std::int64_t product_id = 0;
    TransactionType type = TransactionType::Income;
    double quantity = 0.0;
    std::chrono::system_clock::time_point at{};
    std::string comment;
};

}
