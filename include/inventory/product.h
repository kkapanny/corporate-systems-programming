#pragma once

#include <cstdint>
#include <string>

namespace inventory {

struct Product {
    std::int64_t id = 0;
    std::string sku;
    std::string name;
    std::string unit;
    double min_stock = 0.0;
    double current_stock = 0.0;
    bool active = true;
};

}
