// обработка ошибочных операций (некорректный ввод и превышение остатка)
#include "helpers/scenario_runner.h"
#include "helpers/test_helpers.h"
#include "inventory/inventory_facade.h"
#include "inventory/memory_store.h"

int main() {
    inventory::InMemoryStore store;
    inventory::InventoryFacade facade(store);
    const auto p = test_helpers::add_sample(store, "ERR", 1.0, 3.0);

    if (facade.post_income(p.id, 0.0, "")) {
        return scenario_fail("приход с нулевым количеством должен отклоняться");
    }
    if (facade.post_expense(p.id, -2.0, "")) {
        return scenario_fail("расход с отрицательным количеством должен отклоняться");
    }
    if (facade.post_expense(p.id, 100.0, "")) {
        return scenario_fail("расход больше остатка должен отклоняться");
    }
    if (facade.post_income(99999, 1.0, "")) {
        return scenario_fail("приход по несуществующему товару должен отклоняться");
    }

    const auto unchanged = store.find_product_by_id(p.id);
    if (!unchanged || unchanged->current_stock != 3.0) {
        return scenario_fail("остаток не должен меняться после ошибок");
    }
    if (!store.transactions_for_product(p.id).empty()) {
        return scenario_fail("ошибочные операции не должны создавать транзакции");
    }

    return scenario_pass("сценарий 07: все ошибочные операции корректно отклонены");
}
