// полный цикл работы склада — добавление, приход, расход, заказ
#include "helpers/scenario_runner.h"
#include "helpers/test_helpers.h"
#include "inventory/inventory_facade.h"
#include "inventory/memory_store.h"

int main() {
    inventory::InMemoryStore store;
    inventory::InventoryFacade facade(store);

    auto p = test_helpers::make_product("FULL", "Полный цикл", 20.0, 0.0);
    p = store.add_product(p);

    if (!facade.post_income(p.id, 25.0, "начальный приход")) {
        return scenario_fail("шаг прихода не выполнен");
    }
    if (!facade.post_expense(p.id, 10.0, "отгрузка")) {
        return scenario_fail("шаг расхода не выполнен");
    }

    const auto after_ops = store.find_product_by_id(p.id);
    if (!after_ops || after_ops->current_stock != 15.0) {
        return scenario_fail("остаток после операций должен быть 15");
    }

    // current=15 < min=20 => need 5
    const auto order = facade.build_draft_replenishment_order();
    if (order.items.size() != 1 || order.items[0].quantity != 5.0) {
        return scenario_fail("неверный расчёт дефицита в заказе (ожидалось qty=5)");
    }

    if (store.all_transactions().size() < 2 || store.all_orders().size() < 1) {
        return scenario_fail("история транзакций или заказов не сохранена");
    }

    return scenario_pass("сценарий 06: полный цикл завершён, остаток=15, заказ на 5");
}
