#pragma once

#include "inventory/order.h"
#include "inventory/product.h"
#include "inventory/transaction.h"

#include <optional>
#include <vector>

namespace inventory {

class InMemoryStore {
public:
    std::optional<Product> find_product_by_id(std::int64_t id) const;
    std::optional<Product> find_product_by_sku(const std::string& sku) const;
    std::vector<Product> all_products() const;

    Product add_product(const Product& p);
    bool update_product(const Product& p);

    StockTransaction add_transaction(const StockTransaction& tx);
    std::vector<StockTransaction> transactions_for_product(std::int64_t product_id) const;
    std::vector<StockTransaction> all_transactions() const;

    ReplenishmentOrder add_order(const ReplenishmentOrder& o);
    std::vector<ReplenishmentOrder> all_orders() const;

private:
    std::vector<Product> products_;
    std::vector<StockTransaction> transactions_;
    std::vector<ReplenishmentOrder> orders_;

    std::int64_t next_product_id_ = 1;
    std::int64_t next_tx_id_ = 1;
    std::int64_t next_order_id_ = 1;
};

}
