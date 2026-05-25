// проведение прихода (аналог пункта меню «3»)
#include "helpers/scenario_runner.h"
#include "helpers/test_helpers.h"
#include "inventory/inventory_facade.h"
#include "inventory/memory_store.h"
#include "inventory/transaction.h"

int main() {
    inventory::InMemoryStore store;
    inventory::InventoryFacade facade(store);
    const auto p = test_helpers::add_sample(store, "IN-FLOW", 5.0, 2.0);

    if (!facade.post_income(p.id, 8.0, "сценарий-приход")) {
        return scenario_fail("post_income вернул false");
    }

    const auto updated = store.find_product_by_id(p.id);
    if (!updated || updated->current_stock != 10.0) {
        return scenario_fail("остаток после прихода должен быть 10");
    }

    const auto txs = store.transactions_for_product(p.id);
    if (txs.empty() || txs.back().type != inventory::TransactionType::Income) {
        return scenario_fail("транзакция прихода не записана");
    }

    return scenario_pass("сценарий 03: приход +8, остаток=10");
}
