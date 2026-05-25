#include "inventory/inventory_facade.h"

#include <chrono>

namespace inventory {

namespace {
StockTransaction make_tx(std::int64_t product_id, TransactionType type, double qty,
                         const std::string& comment) {
    StockTransaction tx;
    tx.product_id = product_id;
    tx.type = type;
    tx.quantity = qty;
    tx.at = std::chrono::system_clock::now();
    tx.comment = comment;
    return tx;
}
}

bool InventoryFacade::post_income(std::int64_t product_id, double qty, const std::string& comment) {
    if (qty <= 0.0) {
        return false;
    }
    auto p = store_.find_product_by_id(product_id);
    if (!p || !p->active) {
        return false;
    }
    Product upd = *p;
    upd.current_stock += qty;
    if (!store_.update_product(upd)) {
        return false;
    }
    store_.add_transaction(make_tx(product_id, TransactionType::Income, qty, comment));
    return true;
}

bool InventoryFacade::post_expense(std::int64_t product_id, double qty, const std::string& comment) {
    if (qty <= 0.0) {
        return false;
    }
    auto p = store_.find_product_by_id(product_id);
    if (!p || !p->active) {
        return false;
    }
    if (p->current_stock < qty) {
        return false;
    }
    Product upd = *p;
    upd.current_stock -= qty;
    if (!store_.update_product(upd)) {
        return false;
    }
    store_.add_transaction(make_tx(product_id, TransactionType::Expense, qty, comment));
    return true;
}

ReplenishmentOrder InventoryFacade::build_draft_replenishment_order() {
    ReplenishmentOrder order;
    order.created_at = std::chrono::system_clock::now();
    order.status = "draft";

    for (const auto& p : store_.all_products()) {
        if (!p.active) {
            continue;
        }
        if (p.current_stock < p.min_stock) {
            const double need = p.min_stock - p.current_stock;
            if (need > 0.0) {
                order.items.push_back(OrderItem{p.id, need});
            }
        }
    }
    const ReplenishmentOrder saved = store_.add_order(order);
    return saved;
}

}
