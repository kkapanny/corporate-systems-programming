// проведение расхода (аналог пункта меню «4»)
#include "helpers/scenario_runner.h"
#include "helpers/test_helpers.h"
#include "inventory/inventory_facade.h"
#include "inventory/memory_store.h"
#include "inventory/transaction.h"

int main() {
    inventory::InMemoryStore store;
    inventory::InventoryFacade facade(store);
    const auto p = test_helpers::add_sample(store, "OUT-FLOW", 1.0, 12.0);

    if (!facade.post_expense(p.id, 5.0, "сценарий-расход")) {
        return scenario_fail("post_expense вернул false");
    }

    const auto updated = store.find_product_by_id(p.id);
    if (!updated || updated->current_stock != 7.0) {
        return scenario_fail("остаток после расхода должен быть 7");
    }

    const auto txs = store.transactions_for_product(p.id);
    if (txs.empty() || txs.back().type != inventory::TransactionType::Expense) {
        return scenario_fail("транзакция расхода не записана");
    }

    return scenario_pass("сценарий 04: расход -5, остаток=7");
}
