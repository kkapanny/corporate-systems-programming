// черновик заказа на пополнение при дефиците (пункт меню «5»)
#include "helpers/scenario_runner.h"
#include "helpers/test_helpers.h"
#include "inventory/inventory_facade.h"
#include "inventory/memory_store.h"

int main() {
    inventory::InMemoryStore store;
    inventory::InventoryFacade facade(store);

    const auto deficit = test_helpers::add_sample(store, "LOW", 30.0, 8.0);
    test_helpers::add_sample(store, "OK", 5.0, 50.0);

    const auto order = facade.build_draft_replenishment_order();
    if (order.status != "draft") {
        return scenario_fail("статус заказа должен быть draft");
    }
    if (order.items.size() != 1) {
        return scenario_fail("в заказе должна быть одна позиция дефицита");
    }
    if (order.items[0].product_id != deficit.id || order.items[0].quantity != 22.0) {
        return scenario_fail("неверное количество к пополнению (ожидалось 22)");
    }

    return scenario_pass("сценарий 05: черновик заказа id=" + std::to_string(order.id) +
                         ", позиций=1, qty=22");
}
