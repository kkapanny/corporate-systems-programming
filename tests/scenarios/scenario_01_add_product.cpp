// добавление товара на склад (аналог пункта меню «1»)
#include "helpers/scenario_runner.h"
#include "helpers/test_helpers.h"
#include "inventory/memory_store.h"

int main() {
    inventory::InMemoryStore store;

    auto p = test_helpers::make_product("SCN-01", "Демо-товар", 15.0, 0.0);
    p.unit = "шт";
    const auto added = store.add_product(p);

    if (added.id < 1) {
        return scenario_fail("товар не получил id");
    }
    const auto found = store.find_product_by_sku("SCN-01");
    if (!found || found->name != "Демо-товар") {
        return scenario_fail("товар не найден по SKU после добавления");
    }
    if (found->min_stock != 15.0 || found->current_stock != 0.0) {
        return scenario_fail("неверные остатки у нового товара");
    }

    return scenario_pass("сценарий 01: товар успешно добавлен, id=" + std::to_string(added.id));
}
