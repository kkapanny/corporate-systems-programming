#include "inventory/memory_store.h"

#include <algorithm>

namespace inventory {

std::optional<Product> InMemoryStore::find_product_by_id(std::int64_t id) const {
    auto it = std::find_if(products_.begin(), products_.end(),
                           [id](const Product& p) { return p.id == id; });
    if (it == products_.end()) {
        return std::nullopt;
    }
    return *it;
}

std::optional<Product> InMemoryStore::find_product_by_sku(const std::string& sku) const {
    auto it = std::find_if(products_.begin(), products_.end(),
                           [&sku](const Product& p) { return p.sku == sku; });
    if (it == products_.end()) {
        return std::nullopt;
    }
    return *it;
}

std::vector<Product> InMemoryStore::all_products() const { return products_; }

Product InMemoryStore::add_product(const Product& p) {
    Product copy = p;
    copy.id = next_product_id_++;
    products_.push_back(std::move(copy));
    return products_.back();
}

bool InMemoryStore::update_product(const Product& p) {
    auto it = std::find_if(products_.begin(), products_.end(),
                           [&p](const Product& x) { return x.id == p.id; });
    if (it == products_.end()) {
        return false;
    }
    *it = p;
    return true;
}

StockTransaction InMemoryStore::add_transaction(const StockTransaction& tx) {
    StockTransaction copy = tx;
    copy.id = next_tx_id_++;
    transactions_.push_back(std::move(copy));
    return transactions_.back();
}

std::vector<StockTransaction> InMemoryStore::transactions_for_product(std::int64_t product_id) const {
    std::vector<StockTransaction> out;
    for (const auto& t : transactions_) {
        if (t.product_id == product_id) {
            out.push_back(t);
        }
    }
    return out;
}

std::vector<StockTransaction> InMemoryStore::all_transactions() const { return transactions_; }

ReplenishmentOrder InMemoryStore::add_order(const ReplenishmentOrder& o) {
    ReplenishmentOrder copy = o;
    copy.id = next_order_id_++;
    orders_.push_back(std::move(copy));
    return orders_.back();
}

std::vector<ReplenishmentOrder> InMemoryStore::all_orders() const { return orders_; }

}
