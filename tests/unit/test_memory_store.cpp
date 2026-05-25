#include "helpers/test_helpers.h"
#include "inventory/memory_store.h"
#include "inventory/order.h"
#include "inventory/transaction.h"

#include <gtest/gtest.h>

using inventory::InMemoryStore;
using inventory::ReplenishmentOrder;
using inventory::StockTransaction;
using inventory::TransactionType;
using test_helpers::add_sample;
using test_helpers::make_product;

class InMemoryStoreTest : public ::testing::Test {
protected:
    InMemoryStore store;
};

// найти продукт по id (7 тестов)

TEST_F(InMemoryStoreTest, FindProductByIdReturnsNulloptWhenEmpty) {
    EXPECT_FALSE(store.find_product_by_id(1).has_value());
}

TEST_F(InMemoryStoreTest, FindProductByIdReturnsProductWhenExists) {
    const auto added = add_sample(store, "A1", 5.0);
    const auto found = store.find_product_by_id(added.id);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->sku, "A1");
}

TEST_F(InMemoryStoreTest, FindProductByIdReturnsNulloptForUnknownId) {
    add_sample(store, "A2", 1.0);
    EXPECT_FALSE(store.find_product_by_id(999).has_value());
}

TEST_F(InMemoryStoreTest, FindProductByIdDistinguishesMultipleProducts) {
    const auto p1 = add_sample(store, "X", 1.0);
    add_sample(store, "Y", 2.0);
    const auto found = store.find_product_by_id(p1.id);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->sku, "X");
}

TEST_F(InMemoryStoreTest, FindProductByIdAfterUpdateReflectsChanges) {
    auto p = add_sample(store, "U1", 3.0);
    p.current_stock = 50.0;
    ASSERT_TRUE(store.update_product(p));
    const auto found = store.find_product_by_id(p.id);
    ASSERT_TRUE(found.has_value());
    EXPECT_DOUBLE_EQ(found->current_stock, 50.0);
}

TEST_F(InMemoryStoreTest, FindProductByIdZeroIdNotFoundUnlessAssigned) {
    add_sample(store, "Z", 1.0);
    EXPECT_FALSE(store.find_product_by_id(0).has_value());
}

TEST_F(InMemoryStoreTest, FindProductByIdNegativeIdNotFound) {
    add_sample(store, "N", 1.0);
    EXPECT_FALSE(store.find_product_by_id(-1).has_value());
}

// найти продукт по sku (7 тестов)

TEST_F(InMemoryStoreTest, FindProductBySkuReturnsNulloptWhenEmpty) {
    EXPECT_FALSE(store.find_product_by_sku("ANY").has_value());
}

TEST_F(InMemoryStoreTest, FindProductBySkuReturnsMatch) {
    add_sample(store, "SKU-100", 2.0);
    const auto found = store.find_product_by_sku("SKU-100");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name, "Товар SKU-100");
}

TEST_F(InMemoryStoreTest, FindProductBySkuCaseSensitive) {
    add_sample(store, "AbC", 1.0);
    EXPECT_FALSE(store.find_product_by_sku("abc").has_value());
    EXPECT_TRUE(store.find_product_by_sku("AbC").has_value());
}

TEST_F(InMemoryStoreTest, FindProductBySkuUnknownReturnsNullopt) {
    add_sample(store, "EXIST", 1.0);
    EXPECT_FALSE(store.find_product_by_sku("MISSING").has_value());
}

TEST_F(InMemoryStoreTest, FindProductBySkuEmptyStringNotFoundUnlessStored) {
    auto p = make_product("", "Пустой SKU", 0.0);
    store.add_product(p);
    EXPECT_TRUE(store.find_product_by_sku("").has_value());
}

TEST_F(InMemoryStoreTest, FindProductBySkuReturnsFirstWhenDuplicates) {
    const auto first = add_sample(store, "DUP", 1.0);
    add_sample(store, "DUP", 2.0);
    const auto found = store.find_product_by_sku("DUP");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->id, first.id);
}

TEST_F(InMemoryStoreTest, FindProductBySkuAfterRenameViaUpdate) {
    auto p = add_sample(store, "OLD-SKU", 1.0);
    p.sku = "NEW-SKU";
    ASSERT_TRUE(store.update_product(p));
    EXPECT_FALSE(store.find_product_by_sku("OLD-SKU").has_value());
    EXPECT_TRUE(store.find_product_by_sku("NEW-SKU").has_value());
}

// все продукты (6 тестов)

TEST_F(InMemoryStoreTest, AllProductsEmptyInitially) {
    EXPECT_TRUE(store.all_products().empty());
}

TEST_F(InMemoryStoreTest, AllProductsReturnsSingleItem) {
    add_sample(store, "S1", 1.0);
    EXPECT_EQ(store.all_products().size(), 1u);
}

TEST_F(InMemoryStoreTest, AllProductsReturnsCopyNotReference) {
    auto list = store.all_products();
    list.clear();
    EXPECT_EQ(store.all_products().size(), 0u);
}

TEST_F(InMemoryStoreTest, AllProductsContainsAllAdded) {
    add_sample(store, "P1", 1.0);
    add_sample(store, "P2", 2.0);
    add_sample(store, "P3", 3.0);
    EXPECT_EQ(store.all_products().size(), 3u);
}

TEST_F(InMemoryStoreTest, AllProductsReflectsStockAfterUpdate) {
    auto p = add_sample(store, "ST", 5.0, 1.0);
    p.current_stock = 9.0;
    store.update_product(p);
    const auto list = store.all_products();
    ASSERT_EQ(list.size(), 1u);
    EXPECT_DOUBLE_EQ(list[0].current_stock, 9.0);
}

TEST_F(InMemoryStoreTest, AllProductsPreservesInsertionOrder) {
    const auto a = add_sample(store, "FIRST", 1.0);
    const auto b = add_sample(store, "SECOND", 1.0);
    const auto list = store.all_products();
    ASSERT_EQ(list.size(), 2u);
    EXPECT_EQ(list[0].id, a.id);
    EXPECT_EQ(list[1].id, b.id);
}

// добавить продукт (7 тестов)

TEST_F(InMemoryStoreTest, AddProductAssignsIncrementalIds) {
    const auto p1 = add_sample(store, "I1", 1.0);
    const auto p2 = add_sample(store, "I2", 1.0);
    EXPECT_EQ(p2.id, p1.id + 1);
}

TEST_F(InMemoryStoreTest, AddProductPreservesSkuAndName) {
    auto raw = make_product("KEEP", "Имя", 4.0, 0.0);
    raw.name = "Сохранённое имя";
    const auto added = store.add_product(raw);
    EXPECT_EQ(added.sku, "KEEP");
    EXPECT_EQ(added.name, "Сохранённое имя");
}

TEST_F(InMemoryStoreTest, AddProductPreservesMinAndCurrentStock) {
    auto raw = make_product("STK", "S", 12.5, 3.3);
    const auto added = store.add_product(raw);
    EXPECT_DOUBLE_EQ(added.min_stock, 12.5);
    EXPECT_DOUBLE_EQ(added.current_stock, 3.3);
}

TEST_F(InMemoryStoreTest, AddProductPreservesActiveFlag) {
    auto raw = make_product("OFF", "S", 1.0, 0.0, false);
    const auto added = store.add_product(raw);
    EXPECT_FALSE(added.active);
}

TEST_F(InMemoryStoreTest, AddProductIgnoresIncomingId) {
    auto raw = make_product("ID", "S", 1.0);
    raw.id = 5000;
    const auto added = store.add_product(raw);
    EXPECT_NE(added.id, 5000);
    EXPECT_EQ(added.id, 1);
}

TEST_F(InMemoryStoreTest, AddProductReturnsStoredCopyWithAssignedId) {
    const auto added = add_sample(store, "RET", 2.0);
    const auto found = store.find_product_by_id(added.id);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->sku, added.sku);
}

TEST_F(InMemoryStoreTest, AddProductUnitFieldPreserved) {
    auto raw = make_product("U", "S", 1.0);
    raw.unit = "л";
    const auto added = store.add_product(raw);
    EXPECT_EQ(added.unit, "л");
}

// обновить продукт (7 тестов)

TEST_F(InMemoryStoreTest, UpdateProductReturnsFalseForUnknownId) {
    auto p = make_product("X", "Y", 1.0);
    p.id = 404;
    EXPECT_FALSE(store.update_product(p));
}

TEST_F(InMemoryStoreTest, UpdateProductReturnsTrueForExisting) {
    auto p = add_sample(store, "UP", 1.0);
    p.name = "Новое имя";
    EXPECT_TRUE(store.update_product(p));
}

TEST_F(InMemoryStoreTest, UpdateProductChangesNameInStore) {
    auto p = add_sample(store, "NM", 1.0);
    p.name = "Обновлено";
    store.update_product(p);
    const auto found = store.find_product_by_id(p.id);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name, "Обновлено");
}

TEST_F(InMemoryStoreTest, UpdateProductChangesCurrentStock) {
    auto p = add_sample(store, "CS", 10.0, 2.0);
    p.current_stock = 20.0;
    store.update_product(p);
    const auto found = store.find_product_by_id(p.id);
    ASSERT_TRUE(found.has_value());
    EXPECT_DOUBLE_EQ(found->current_stock, 20.0);
}

TEST_F(InMemoryStoreTest, UpdateProductCanDeactivate) {
    auto p = add_sample(store, "ACT", 1.0);
    p.active = false;
    store.update_product(p);
    const auto found = store.find_product_by_id(p.id);
    ASSERT_TRUE(found.has_value());
    EXPECT_FALSE(found->active);
}

TEST_F(InMemoryStoreTest, UpdateProductDoesNotChangeId) {
    const auto p = add_sample(store, "IDK", 1.0);
    auto copy = p;
    copy.sku = "CHANGED";
    store.update_product(copy);
    const auto found = store.find_product_by_id(p.id);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->id, p.id);
}

TEST_F(InMemoryStoreTest, UpdateProductDoesNotAffectOtherProducts) {
    const auto p1 = add_sample(store, "A", 1.0);
    auto p2 = add_sample(store, "B", 1.0);
    p2.name = "B-new";
    store.update_product(p2);
    const auto unchanged = store.find_product_by_id(p1.id);
    ASSERT_TRUE(unchanged.has_value());
    EXPECT_EQ(unchanged->name, "Товар A");
}

// добавить транзакцию (6 тестов)

TEST_F(InMemoryStoreTest, AddTransactionAssignsIncrementalIds) {
    StockTransaction t1;
    t1.product_id = 1;
    t1.quantity = 1.0;
    const auto a = store.add_transaction(t1);
    StockTransaction t2;
    t2.product_id = 1;
    t2.quantity = 2.0;
    const auto b = store.add_transaction(t2);
    EXPECT_EQ(b.id, a.id + 1);
}

TEST_F(InMemoryStoreTest, AddTransactionPreservesTypeAndComment) {
    StockTransaction tx;
    tx.product_id = 5;
    tx.type = TransactionType::Expense;
    tx.quantity = 3.5;
    tx.comment = "списание";
    const auto saved = store.add_transaction(tx);
    EXPECT_EQ(saved.type, TransactionType::Expense);
    EXPECT_EQ(saved.comment, "списание");
    EXPECT_DOUBLE_EQ(saved.quantity, 3.5);
}

TEST_F(InMemoryStoreTest, AddTransactionIgnoresIncomingId) {
    StockTransaction tx;
    tx.id = 999;
    tx.product_id = 1;
    const auto saved = store.add_transaction(tx);
    EXPECT_EQ(saved.id, 1);
}

TEST_F(InMemoryStoreTest, AddTransactionStoresProductId) {
    StockTransaction tx;
    tx.product_id = 42;
    const auto saved = store.add_transaction(tx);
    EXPECT_EQ(saved.product_id, 42);
}

TEST_F(InMemoryStoreTest, AddTransactionMultipleForSameProduct) {
    StockTransaction tx;
    tx.product_id = 7;
    store.add_transaction(tx);
    store.add_transaction(tx);
    EXPECT_EQ(store.transactions_for_product(7).size(), 2u);
}

TEST_F(InMemoryStoreTest, AddTransactionTimestampPreservedIfSet) {
    StockTransaction tx;
    const auto ts = std::chrono::system_clock::now();
    tx.at = ts;
    const auto saved = store.add_transaction(tx);
    EXPECT_EQ(saved.at, ts);
}

// транзакции для продукта (7 тестов)

TEST_F(InMemoryStoreTest, TransactionsForProductEmptyWhenNone) {
    EXPECT_TRUE(store.transactions_for_product(1).empty());
}

TEST_F(InMemoryStoreTest, TransactionsForProductFiltersByProductId) {
    StockTransaction t1;
    t1.product_id = 1;
    StockTransaction t2;
    t2.product_id = 2;
    store.add_transaction(t1);
    store.add_transaction(t2);
    EXPECT_EQ(store.transactions_for_product(1).size(), 1u);
}

TEST_F(InMemoryStoreTest, TransactionsForProductReturnsAllForProduct) {
    StockTransaction tx;
    tx.product_id = 3;
    store.add_transaction(tx);
    store.add_transaction(tx);
    store.add_transaction(tx);
    EXPECT_EQ(store.transactions_for_product(3).size(), 3u);
}

TEST_F(InMemoryStoreTest, TransactionsForProductUnknownIdEmpty) {
    StockTransaction tx;
    tx.product_id = 1;
    store.add_transaction(tx);
    EXPECT_TRUE(store.transactions_for_product(100).empty());
}

TEST_F(InMemoryStoreTest, TransactionsForProductPreservesOrder) {
    StockTransaction t1;
    t1.product_id = 5;
    t1.quantity = 1.0;
    StockTransaction t2;
    t2.product_id = 5;
    t2.quantity = 2.0;
    const auto a = store.add_transaction(t1);
    const auto b = store.add_transaction(t2);
    const auto list = store.transactions_for_product(5);
    ASSERT_EQ(list.size(), 2u);
    EXPECT_EQ(list[0].id, a.id);
    EXPECT_EQ(list[1].id, b.id);
}

TEST_F(InMemoryStoreTest, TransactionsForProductDoesNotIncludeOtherProducts) {
    StockTransaction a;
    a.product_id = 10;
    StockTransaction b;
    b.product_id = 11;
    store.add_transaction(a);
    store.add_transaction(b);
    const auto list = store.transactions_for_product(10);
    ASSERT_EQ(list.size(), 1u);
    EXPECT_EQ(list[0].product_id, 10);
}

TEST_F(InMemoryStoreTest, TransactionsForProductNegativeIdEmpty) {
    StockTransaction tx;
    tx.product_id = 1;
    store.add_transaction(tx);
    EXPECT_TRUE(store.transactions_for_product(-5).empty());
}

// все транзакции (5 тестов)

TEST_F(InMemoryStoreTest, AllTransactionsEmptyInitially) {
    EXPECT_TRUE(store.all_transactions().empty());
}

TEST_F(InMemoryStoreTest, AllTransactionsReturnsAllAdded) {
    store.add_transaction(StockTransaction{});
    store.add_transaction(StockTransaction{});
    EXPECT_EQ(store.all_transactions().size(), 2u);
}

TEST_F(InMemoryStoreTest, AllTransactionsIsCopy) {
    auto list = store.all_transactions();
    list.clear();
    EXPECT_EQ(store.all_transactions().size(), 0u);
}

TEST_F(InMemoryStoreTest, AllTransactionsMixedProducts) {
    StockTransaction t1;
    t1.product_id = 1;
    StockTransaction t2;
    t2.product_id = 2;
    store.add_transaction(t1);
    store.add_transaction(t2);
    EXPECT_EQ(store.all_transactions().size(), 2u);
}

TEST_F(InMemoryStoreTest, AllTransactionsReflectsLatestAdd) {
    StockTransaction tx;
    tx.comment = "last";
    store.add_transaction(tx);
    const auto list = store.all_transactions();
    ASSERT_EQ(list.size(), 1u);
    EXPECT_EQ(list.back().comment, "last");
}

// добавить заказ (6 тестов)

TEST_F(InMemoryStoreTest, AddOrderAssignsIncrementalIds) {
    ReplenishmentOrder o1;
    o1.status = "draft";
    const auto a = store.add_order(o1);
    ReplenishmentOrder o2;
    o2.status = "sent";
    const auto b = store.add_order(o2);
    EXPECT_EQ(b.id, a.id + 1);
}

TEST_F(InMemoryStoreTest, AddOrderPreservesStatusAndItems) {
    ReplenishmentOrder order;
    order.status = "draft";
    order.items.push_back({1, 5.0});
    const auto saved = store.add_order(order);
    EXPECT_EQ(saved.status, "draft");
    ASSERT_EQ(saved.items.size(), 1u);
    EXPECT_DOUBLE_EQ(saved.items[0].quantity, 5.0);
}

TEST_F(InMemoryStoreTest, AddOrderIgnoresIncomingId) {
    ReplenishmentOrder order;
    order.id = 777;
    const auto saved = store.add_order(order);
    EXPECT_EQ(saved.id, 1);
}

TEST_F(InMemoryStoreTest, AddOrderEmptyItemsAllowed) {
    ReplenishmentOrder order;
    order.status = "empty";
    const auto saved = store.add_order(order);
    EXPECT_TRUE(saved.items.empty());
}

TEST_F(InMemoryStoreTest, AddOrderPreservesCreatedAt) {
    ReplenishmentOrder order;
    const auto ts = std::chrono::system_clock::now();
    order.created_at = ts;
    const auto saved = store.add_order(order);
    EXPECT_EQ(saved.created_at, ts);
}

TEST_F(InMemoryStoreTest, AddOrderMultipleItems) {
    ReplenishmentOrder order;
    order.items = {{1, 1.0}, {2, 2.0}, {3, 3.0}};
    const auto saved = store.add_order(order);
    EXPECT_EQ(saved.items.size(), 3u);
}

// все заказы (5 тестов)

TEST_F(InMemoryStoreTest, AllOrdersEmptyInitially) {
    EXPECT_TRUE(store.all_orders().empty());
}

TEST_F(InMemoryStoreTest, AllOrdersReturnsAll) {
    store.add_order(ReplenishmentOrder{});
    store.add_order(ReplenishmentOrder{});
    EXPECT_EQ(store.all_orders().size(), 2u);
}

TEST_F(InMemoryStoreTest, AllOrdersIsCopy) {
    auto list = store.all_orders();
    list.clear();
    EXPECT_EQ(store.all_orders().size(), 0u);
}

TEST_F(InMemoryStoreTest, AllOrdersPreservesInsertionOrder) {
    ReplenishmentOrder o1;
    o1.status = "first";
    ReplenishmentOrder o2;
    o2.status = "second";
    const auto a = store.add_order(o1);
    const auto b = store.add_order(o2);
    const auto list = store.all_orders();
    ASSERT_EQ(list.size(), 2u);
    EXPECT_EQ(list[0].id, a.id);
    EXPECT_EQ(list[1].id, b.id);
}

TEST_F(InMemoryStoreTest, AllOrdersReflectsLatestStatus) {
    ReplenishmentOrder order;
    order.status = "draft";
    store.add_order(order);
    const auto list = store.all_orders();
    ASSERT_EQ(list.size(), 1u);
    EXPECT_EQ(list[0].status, "draft");
}
