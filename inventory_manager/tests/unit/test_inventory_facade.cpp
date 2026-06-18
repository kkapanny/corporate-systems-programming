#include "helpers/test_helpers.h"
#include "inventory/inventory_facade.h"
#include "inventory/memory_store.h"
#include "inventory/transaction.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using Catch::Approx;

using inventory::InventoryFacade;
using inventory::InMemoryStore;
using inventory::TransactionType;
using test_helpers::add_sample;

class InventoryFacadeTest {
public:
    InMemoryStore store;
    InventoryFacade facade{store};
};

// проведение прихода (7 тестов)

// при успешном post_income, current_stock должен увеличиться на количество прихода
TEST_CASE_METHOD(InventoryFacadeTest, "PostIncomeIncreasesStock", "[InventoryFacadeTest]") {
    const auto p = add_sample(store, "INC1", 10.0, 5.0);
    REQUIRE(facade.post_income(p.id, 3.0, "поставка"));
    const auto updated = store.find_product_by_id(p.id);
    REQUIRE(updated.has_value());
    REQUIRE(updated->current_stock == Approx(8.0));
}

// при успешном post_income, в хранилище должна появиться транзакция типа Income
TEST_CASE_METHOD(InventoryFacadeTest, "PostIncomeCreatesIncomeTransaction", "[InventoryFacadeTest]") {
    const auto p = add_sample(store, "INC2", 1.0, 0.0);
    REQUIRE(facade.post_income(p.id, 2.5, "тест"));
    const auto txs = store.transactions_for_product(p.id);
    REQUIRE(txs.size() == 1u);
    REQUIRE(txs[0].type == TransactionType::Income);
    REQUIRE(txs[0].quantity == Approx(2.5));
    REQUIRE(txs[0].comment == "тест");
}

// при post_income с нулевым количеством, операция должна быть отклонена
TEST_CASE_METHOD(InventoryFacadeTest, "PostIncomeRejectsZeroQuantity", "[InventoryFacadeTest]") {
    const auto p = add_sample(store, "INC3", 1.0);
    REQUIRE_FALSE(facade.post_income(p.id, 0.0, ""));
}

// при post_income с отрицательным количеством, операция должна быть отклонена
TEST_CASE_METHOD(InventoryFacadeTest, "PostIncomeRejectsNegativeQuantity", "[InventoryFacadeTest]") {
    const auto p = add_sample(store, "INC4", 1.0);
    REQUIRE_FALSE(facade.post_income(p.id, -1.0, ""));
}

// при post_income для несуществующего product_id, операция должна быть отклонена
TEST_CASE_METHOD(InventoryFacadeTest, "PostIncomeRejectsUnknownProduct", "[InventoryFacadeTest]") {
    REQUIRE_FALSE(facade.post_income(9999, 1.0, ""));
}

// при post_income для неактивного товара, операция должна быть отклонена
TEST_CASE_METHOD(InventoryFacadeTest, "PostIncomeRejectsInactiveProduct", "[InventoryFacadeTest]") {
    const auto p = add_sample(store, "INC5", 1.0, 0.0, false);
    REQUIRE_FALSE(facade.post_income(p.id, 1.0, ""));
    const auto unchanged = store.find_product_by_id(p.id);
    REQUIRE(unchanged.has_value());
    REQUIRE(unchanged->current_stock == Approx(0.0));
}

// при нескольких успешных post_income, остаток и число транзакций должны накапливаться
TEST_CASE_METHOD(InventoryFacadeTest, "PostIncomeAccumulatesMultipleTimes", "[InventoryFacadeTest]") {
    const auto p = add_sample(store, "INC6", 100.0, 0.0);
    REQUIRE(facade.post_income(p.id, 4.0, "a"));
    REQUIRE(facade.post_income(p.id, 6.0, "b"));
    const auto updated = store.find_product_by_id(p.id);
    REQUIRE(updated.has_value());
    REQUIRE(updated->current_stock == Approx(10.0));
    REQUIRE(store.transactions_for_product(p.id).size() == 2u);
}

// проведение расхода (7 тестов)

// при успешном post_expense, current_stock должен уменьшиться на количество расхода
TEST_CASE_METHOD(InventoryFacadeTest, "PostExpenseDecreasesStock", "[InventoryFacadeTest]") {
    const auto p = add_sample(store, "EXP1", 1.0, 10.0);
    REQUIRE(facade.post_expense(p.id, 4.0, "расход"));
    const auto updated = store.find_product_by_id(p.id);
    REQUIRE(updated.has_value());
    REQUIRE(updated->current_stock == Approx(6.0));
}

// при успешном post_expense, в хранилище должна появиться транзакция типа Expense
TEST_CASE_METHOD(InventoryFacadeTest, "PostExpenseCreatesExpenseTransaction", "[InventoryFacadeTest]") {
    const auto p = add_sample(store, "EXP2", 1.0, 5.0);
    REQUIRE(facade.post_expense(p.id, 1.0, "списание"));
    const auto txs = store.transactions_for_product(p.id);
    REQUIRE(txs.size() == 1u);
    REQUIRE(txs[0].type == TransactionType::Expense);
}

// при post_expense с количеством больше остатка, операция должна быть отклонена
TEST_CASE_METHOD(InventoryFacadeTest, "PostExpenseRejectsInsufficientStock", "[InventoryFacadeTest]") {
    const auto p = add_sample(store, "EXP3", 1.0, 2.0);
    REQUIRE_FALSE(facade.post_expense(p.id, 2.01, ""));
    const auto unchanged = store.find_product_by_id(p.id);
    REQUIRE(unchanged.has_value());
    REQUIRE(unchanged->current_stock == Approx(2.0));
}

// при post_expense на весь остаток, операция должна быть успешной и остаток стать 0
TEST_CASE_METHOD(InventoryFacadeTest, "PostExpenseAllowsExactStockWithdrawal", "[InventoryFacadeTest]") {
    const auto p = add_sample(store, "EXP4", 1.0, 7.0);
    REQUIRE(facade.post_expense(p.id, 7.0, ""));
    const auto updated = store.find_product_by_id(p.id);
    REQUIRE(updated.has_value());
    REQUIRE(updated->current_stock == Approx(0.0));
}

// при post_expense с нулевым количеством, операция должна быть отклонена
TEST_CASE_METHOD(InventoryFacadeTest, "PostExpenseRejectsZeroQuantity", "[InventoryFacadeTest]") {
    const auto p = add_sample(store, "EXP5", 1.0, 5.0);
    REQUIRE_FALSE(facade.post_expense(p.id, 0.0, ""));
}

// при post_expense для несуществующего product_id, операция должна быть отклонена
TEST_CASE_METHOD(InventoryFacadeTest, "PostExpenseRejectsUnknownProduct", "[InventoryFacadeTest]") {
    REQUIRE_FALSE(facade.post_expense(12345, 1.0, ""));
}

// при post_expense для неактивного товара, операция должна быть отклонена
TEST_CASE_METHOD(InventoryFacadeTest, "PostExpenseRejectsInactiveProduct", "[InventoryFacadeTest]") {
    const auto p = add_sample(store, "EXP6", 1.0, 10.0, false);
    REQUIRE_FALSE(facade.post_expense(p.id, 1.0, ""));
}

// построить черновик заказа на пополнение (7 тестов)

// при build_draft_replenishment_order, заказ должен иметь status draft и назначенный id
TEST_CASE_METHOD(InventoryFacadeTest, "BuildDraftCreatesOrderWithDraftStatus", "[InventoryFacadeTest]") {
    add_sample(store, "DR1", 10.0, 0.0);
    const auto order = facade.build_draft_replenishment_order();
    REQUIRE(order.status == "draft");
    REQUIRE(order.id >= 1);
}

// при дефиците (current_stock < min_stock), в заказ должна попасть позиция с количеством дефицита
TEST_CASE_METHOD(InventoryFacadeTest, "BuildDraftIncludesDeficitProducts", "[InventoryFacadeTest]") {
    const auto p = add_sample(store, "DR2", 20.0, 5.0);
    const auto order = facade.build_draft_replenishment_order();
    REQUIRE(order.items.size() == 1u);
    REQUIRE(order.items[0].product_id == p.id);
    REQUIRE(order.items[0].quantity == Approx(15.0));
}

// при остатке не ниже min_stock, товар не должен попадать в черновик заказа
TEST_CASE_METHOD(InventoryFacadeTest, "BuildDraftSkipsProductsAtOrAboveMinStock", "[InventoryFacadeTest]") {
    add_sample(store, "OK1", 10.0, 10.0);
    add_sample(store, "OK2", 5.0, 100.0);
    const auto order = facade.build_draft_replenishment_order();
    REQUIRE(order.items.empty());
}

// при неактивном товаре, он не должен попадать в черновик заказа
TEST_CASE_METHOD(InventoryFacadeTest, "BuildDraftSkipsInactiveProducts", "[InventoryFacadeTest]") {
    add_sample(store, "INACT", 50.0, 0.0, false);
    const auto order = facade.build_draft_replenishment_order();
    REQUIRE(order.items.empty());
}

// при нескольких товарах с дефицитом, в заказе должно быть несколько позиций
TEST_CASE_METHOD(InventoryFacadeTest, "BuildDraftMultipleDeficitLines", "[InventoryFacadeTest]") {
    add_sample(store, "M1", 10.0, 0.0);
    add_sample(store, "M2", 5.0, 1.0);
    const auto order = facade.build_draft_replenishment_order();
    REQUIRE(order.items.size() == 2u);
}

// при build_draft_replenishment_order, заказ должен сохраняться в хранилище
TEST_CASE_METHOD(InventoryFacadeTest, "BuildDraftPersistsOrderInStore", "[InventoryFacadeTest]") {
    add_sample(store, "SAVE", 8.0, 2.0);
    const auto order = facade.build_draft_replenishment_order();
    const auto all = store.all_orders();
    REQUIRE(all.size() == 1u);
    REQUIRE(all[0].id == order.id);
}

// при пустом каталоге, черновик должен быть пустым, но со status draft
TEST_CASE_METHOD(InventoryFacadeTest, "BuildDraftEmptyWhenNoProducts", "[InventoryFacadeTest]") {
    const auto order = facade.build_draft_replenishment_order();
    REQUIRE(order.items.empty());
    REQUIRE(order.status == "draft");
}
