#include "inventory/inventory_facade.h"
#include "inventory/memory_store.h"
#include "inventory/product.h"

#include <iostream>
#include <limits>
#include <string>

namespace {

void print_menu() {
    std::cout << "\n--- Склад (промежуточная версия) ---\n"
                 "1) Добавить товар\n"
                 "2) Список товаров\n"
                 "3) Приход\n"
                 "4) Расход\n"
                 "5) Черновик заказа на пополнение (дефицит)\n"
                 "0) Выход\n"
                 "Выбор: ";
}

double read_double(const std::string& prompt) {
    double x = 0.0;
    std::cout << prompt;
    if (!(std::cin >> x)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return -1.0;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return x;
}

std::int64_t read_int64(const std::string& prompt) {
    long long v = 0;
    std::cout << prompt;
    if (!(std::cin >> v)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return -1;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return static_cast<std::int64_t>(v);
}

std::string read_line(const std::string& prompt) {
    std::string s;
    std::cout << prompt;
    std::getline(std::cin, s);
    return s;
}

}

int main() {
    inventory::InMemoryStore store;
    inventory::InventoryFacade facade(store);

    std::cout << "warehouse_mgmt — демо модульной структуры + CMake\n";

    for (;;) {
        print_menu();
        int cmd = 0;
        if (!(std::cin >> cmd)) {
            break;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (cmd == 0) {
            break;
        }
        if (cmd == 1) {
            const std::string sku = read_line("SKU: ");
            const std::string name = read_line("Наименование: ");
            const std::string unit = read_line("Ед. изм.: ");
            const double min_s = read_double("Мин. остаток: ");
            if (min_s < 0.0) {
                std::cout << "Ошибка ввода min.\n";
                continue;
            }
            inventory::Product p;
            p.sku = sku;
            p.name = name;
            p.unit = unit;
            p.min_stock = min_s;
            p.current_stock = 0.0;
            p.active = true;
            const inventory::Product added = store.add_product(p);
            std::cout << "Создан товар id=" << added.id << "\n";
        } else if (cmd == 2) {
            for (const auto& p : store.all_products()) {
                std::cout << "id=" << p.id << " sku=" << p.sku << " name=" << p.name
                          << " current=" << p.current_stock << " min=" << p.min_stock << "\n";
            }
        } else if (cmd == 3) {
            const std::int64_t id = read_int64("id товара: ");
            const double q = read_double("Количество: ");
            if (q <= 0.0) {
                std::cout << "Некорректное количество.\n";
                continue;
            }
            if (facade.post_income(id, q, "консоль")) {
                std::cout << "Приход проведен.\n";
            } else {
                std::cout << "Ошибка прихода (нет товара или ввод).\n";
            }
        } else if (cmd == 4) {
            const std::int64_t id = read_int64("id товара: ");
            const double q = read_double("Количество: ");
            if (q <= 0.0) {
                std::cout << "Некорректное количество.\n";
                continue;
            }
            if (facade.post_expense(id, q, "консоль")) {
                std::cout << "Расход проведен.\n";
            } else {
                std::cout << "Ошибка расхода (остаток, нет товара или ввод).\n";
            }
        } else if (cmd == 5) {
            const auto o = facade.build_draft_replenishment_order();
            std::cout << "Заказ id=" << o.id << " статус=" << o.status
                      << " позиций: " << o.items.size() << "\n";
            for (const auto& it : o.items) {
                std::cout << "  product_id=" << it.product_id << " qty=" << it.quantity << "\n";
            }
        } else {
            std::cout << "Неизвестная команда.\n";
        }
    }

    std::cout << "Выход.\n";
    return 0;
}
