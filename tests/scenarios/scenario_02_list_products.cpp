// просмотр списка товаров (аналог пункта меню «2»)
#include "helpers/scenario_runner.h"
#include "helpers/test_helpers.h"
#include "inventory/memory_store.h"

#include <sstream>

int main() {
    inventory::InMemoryStore store;
    test_helpers::add_sample(store, "L1", 5.0, 1.0);
    test_helpers::add_sample(store, "L2", 10.0, 20.0);

    const auto products = store.all_products();
    if (products.size() != 2) {
        return scenario_fail("ожидалось 2 товара в списке");
    }

    std::ostringstream report;
    for (const auto& p : products) {
        report << p.sku << ':' << p.current_stock << ';';
    }
    const std::string summary = report.str();
    if (summary.find("L1:1") == std::string::npos || summary.find("L2:20") == std::string::npos) {
        return scenario_fail("список не отражает остатки товаров");
    }

    return scenario_pass("сценарий 02: список из 2 товаров — " + summary);
}
