#include "helpers/test_helpers.h"
#include "inventory/inventory_facade.h"
#include "inventory/memory_store.h"
#include "inventory/transaction.h"

#include <gtest/gtest.h>

using inventory::InventoryFacade;
using inventory::InMemoryStore;
using inventory::TransactionType;
using test_helpers::add_sample;

class InventoryFacadeTest : public ::testing::Test {
protected:
    InMemoryStore store;
    InventoryFacade facade{store};
};

// проведение прихода (7 тестов)

TEST_F(InventoryFacadeTest, PostIncomeIncreasesStock) {
    const auto p = add_sample(store, "INC1", 10.0, 5.0);
    ASSERT_TRUE(facade.post_income(p.id, 3.0, "поставка"));
    const auto updated = store.find_product_by_id(p.id);
    ASSERT_TRUE(updated.has_value());
    EXPECT_DOUBLE_EQ(updated->current_stock, 8.0);
}

TEST_F(InventoryFacadeTest, PostIncomeCreatesIncomeTransaction) {
    const auto p = add_sample(store, "INC2", 1.0, 0.0);
    ASSERT_TRUE(facade.post_income(p.id, 2.5, "тест"));
    const auto txs = store.transactions_for_product(p.id);
    ASSERT_EQ(txs.size(), 1u);
    EXPECT_EQ(txs[0].type, TransactionType::Income);
    EXPECT_DOUBLE_EQ(txs[0].quantity, 2.5);
    EXPECT_EQ(txs[0].comment, "тест");
}

TEST_F(InventoryFacadeTest, PostIncomeRejectsZeroQuantity) {
    const auto p = add_sample(store, "INC3", 1.0);
    EXPECT_FALSE(facade.post_income(p.id, 0.0, ""));
}

TEST_F(InventoryFacadeTest, PostIncomeRejectsNegativeQuantity) {
    const auto p = add_sample(store, "INC4", 1.0);
    EXPECT_FALSE(facade.post_income(p.id, -1.0, ""));
}

TEST_F(InventoryFacadeTest, PostIncomeRejectsUnknownProduct) {
    EXPECT_FALSE(facade.post_income(9999, 1.0, ""));
}

TEST_F(InventoryFacadeTest, PostIncomeRejectsInactiveProduct) {
    const auto p = add_sample(store, "INC5", 1.0, 0.0, false);
    EXPECT_FALSE(facade.post_income(p.id, 1.0, ""));
    const auto unchanged = store.find_product_by_id(p.id);
    ASSERT_TRUE(unchanged.has_value());
    EXPECT_DOUBLE_EQ(unchanged->current_stock, 0.0);
}

TEST_F(InventoryFacadeTest, PostIncomeAccumulatesMultipleTimes) {
    const auto p = add_sample(store, "INC6", 100.0, 0.0);
    ASSERT_TRUE(facade.post_income(p.id, 4.0, "a"));
    ASSERT_TRUE(facade.post_income(p.id, 6.0, "b"));
    const auto updated = store.find_product_by_id(p.id);
    ASSERT_TRUE(updated.has_value());
    EXPECT_DOUBLE_EQ(updated->current_stock, 10.0);
    EXPECT_EQ(store.transactions_for_product(p.id).size(), 2u);
}

// проведение расхода (7 тестов)

TEST_F(InventoryFacadeTest, PostExpenseDecreasesStock) {
    const auto p = add_sample(store, "EXP1", 1.0, 10.0);
    ASSERT_TRUE(facade.post_expense(p.id, 4.0, "расход"));
    const auto updated = store.find_product_by_id(p.id);
    ASSERT_TRUE(updated.has_value());
    EXPECT_DOUBLE_EQ(updated->current_stock, 6.0);
}

TEST_F(InventoryFacadeTest, PostExpenseCreatesExpenseTransaction) {
    const auto p = add_sample(store, "EXP2", 1.0, 5.0);
    ASSERT_TRUE(facade.post_expense(p.id, 1.0, "списание"));
    const auto txs = store.transactions_for_product(p.id);
    ASSERT_EQ(txs.size(), 1u);
    EXPECT_EQ(txs[0].type, TransactionType::Expense);
}

TEST_F(InventoryFacadeTest, PostExpenseRejectsInsufficientStock) {
    const auto p = add_sample(store, "EXP3", 1.0, 2.0);
    EXPECT_FALSE(facade.post_expense(p.id, 2.01, ""));
    const auto unchanged = store.find_product_by_id(p.id);
    ASSERT_TRUE(unchanged.has_value());
    EXPECT_DOUBLE_EQ(unchanged->current_stock, 2.0);
}

TEST_F(InventoryFacadeTest, PostExpenseAllowsExactStockWithdrawal) {
    const auto p = add_sample(store, "EXP4", 1.0, 7.0);
    ASSERT_TRUE(facade.post_expense(p.id, 7.0, ""));
    const auto updated = store.find_product_by_id(p.id);
    ASSERT_TRUE(updated.has_value());
    EXPECT_DOUBLE_EQ(updated->current_stock, 0.0);
}

TEST_F(InventoryFacadeTest, PostExpenseRejectsZeroQuantity) {
    const auto p = add_sample(store, "EXP5", 1.0, 5.0);
    EXPECT_FALSE(facade.post_expense(p.id, 0.0, ""));
}

TEST_F(InventoryFacadeTest, PostExpenseRejectsUnknownProduct) {
    EXPECT_FALSE(facade.post_expense(12345, 1.0, ""));
}

TEST_F(InventoryFacadeTest, PostExpenseRejectsInactiveProduct) {
    const auto p = add_sample(store, "EXP6", 1.0, 10.0, false);
    EXPECT_FALSE(facade.post_expense(p.id, 1.0, ""));
}

// построить черновик заказа на пополнение (7 тестов)

TEST_F(InventoryFacadeTest, BuildDraftCreatesOrderWithDraftStatus) {
    add_sample(store, "DR1", 10.0, 0.0);
    const auto order = facade.build_draft_replenishment_order();
    EXPECT_EQ(order.status, "draft");
    EXPECT_GE(order.id, 1);
}

TEST_F(InventoryFacadeTest, BuildDraftIncludesDeficitProducts) {
    const auto p = add_sample(store, "DR2", 20.0, 5.0);
    const auto order = facade.build_draft_replenishment_order();
    ASSERT_EQ(order.items.size(), 1u);
    EXPECT_EQ(order.items[0].product_id, p.id);
    EXPECT_DOUBLE_EQ(order.items[0].quantity, 15.0);
}

TEST_F(InventoryFacadeTest, BuildDraftSkipsProductsAtOrAboveMinStock) {
    add_sample(store, "OK1", 10.0, 10.0);
    add_sample(store, "OK2", 5.0, 100.0);
    const auto order = facade.build_draft_replenishment_order();
    EXPECT_TRUE(order.items.empty());
}

TEST_F(InventoryFacadeTest, BuildDraftSkipsInactiveProducts) {
    add_sample(store, "INACT", 50.0, 0.0, false);
    const auto order = facade.build_draft_replenishment_order();
    EXPECT_TRUE(order.items.empty());
}

TEST_F(InventoryFacadeTest, BuildDraftMultipleDeficitLines) {
    add_sample(store, "M1", 10.0, 0.0);
    add_sample(store, "M2", 5.0, 1.0);
    const auto order = facade.build_draft_replenishment_order();
    EXPECT_EQ(order.items.size(), 2u);
}

TEST_F(InventoryFacadeTest, BuildDraftPersistsOrderInStore) {
    add_sample(store, "SAVE", 8.0, 2.0);
    const auto order = facade.build_draft_replenishment_order();
    const auto all = store.all_orders();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].id, order.id);
}

TEST_F(InventoryFacadeTest, BuildDraftEmptyWhenNoProducts) {
    const auto order = facade.build_draft_replenishment_order();
    EXPECT_TRUE(order.items.empty());
    EXPECT_EQ(order.status, "draft");
}
