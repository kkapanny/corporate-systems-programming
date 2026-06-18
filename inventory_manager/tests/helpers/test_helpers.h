#pragma once

#include "inventory/memory_store.h"
#include "inventory/product.h"

namespace test_helpers {

inline inventory::Product make_product(const std::string& sku, const std::string& name,
                                       double min_stock, double current_stock = 0.0,
                                       bool active = true) {
    inventory::Product p;
    p.sku = sku;
    p.name = name;
    p.unit = "шт";
    p.min_stock = min_stock;
    p.current_stock = current_stock;
    p.active = active;
    return p;
}

inline inventory::Product add_sample(inventory::InMemoryStore& store, const std::string& sku,
                                     double min_stock, double current_stock = 0.0,
                                     bool active = true) {
    return store.add_product(make_product(sku, "Товар " + sku, min_stock, current_stock, active));
}

}
