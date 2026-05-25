#pragma once

#include "inventory/memory_store.h"

#include <string>

namespace inventory {

class InventoryFacade {
public:
    explicit InventoryFacade(InMemoryStore& store) : store_(store) {}

    bool post_income(std::int64_t product_id, double qty, const std::string& comment);
    bool post_expense(std::int64_t product_id, double qty, const std::string& comment);

    ReplenishmentOrder build_draft_replenishment_order();

private:
    InMemoryStore& store_;
};

}
