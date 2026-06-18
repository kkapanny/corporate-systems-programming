#include "helpers/test_helpers.h"
#include "inventory/memory_store.h"
#include "inventory/order.h"
#include "inventory/transaction.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using Catch::Approx;

using inventory::InMemoryStore;
using inventory::ReplenishmentOrder;
using inventory::StockTransaction;
using inventory::TransactionType;
using test_helpers::add_sample;
using test_helpers::make_product;

class InMemoryStoreTest {
public:
    InMemoryStore store;
};

// найти продукт по id (7 тестов)

// при поиске продукта по id, если продукт не найден, должно возвращаться nullopt
TEST_CASE_METHOD(InMemoryStoreTest, "FindProductByIdReturnsNulloptWhenEmpty", "[InMemoryStoreTest]") {
    REQUIRE_FALSE(store.find_product_by_id(1).has_value());
}

// при поиске продукта по id, если продукт найден, должно возвращаться значение
TEST_CASE_METHOD(InMemoryStoreTest, "FindProductByIdReturnsProductWhenExists", "[InMemoryStoreTest]") {
    const auto added = add_sample(store, "A1", 5.0);
    const auto found = store.find_product_by_id(added.id);
    REQUIRE(found.has_value());
    REQUIRE(found->sku == "A1");
}

// при поиске продукта по id, если продукт не найден, должно возвращаться nullopt
TEST_CASE_METHOD(InMemoryStoreTest, "FindProductByIdReturnsNulloptForUnknownId", "[InMemoryStoreTest]") {
    add_sample(store, "A2", 1.0);
    REQUIRE_FALSE(store.find_product_by_id(999).has_value());
}

// при поиске продукта по id, если продукт найден, должно возвращаться значение
TEST_CASE_METHOD(InMemoryStoreTest, "FindProductByIdDistinguishesMultipleProducts", "[InMemoryStoreTest]") {
    const auto p1 = add_sample(store, "X", 1.0);
    add_sample(store, "Y", 2.0);
    const auto found = store.find_product_by_id(p1.id);
    REQUIRE(found.has_value());
    REQUIRE(found->sku == "X");
}

// при обновлении продукта, текущий остаток должен быть сохранен
TEST_CASE_METHOD(InMemoryStoreTest, "FindProductByIdAfterUpdateReflectsChanges", "[InMemoryStoreTest]") {
    auto p = add_sample(store, "U1", 3.0);
    p.current_stock = 50.0;
    REQUIRE(store.update_product(p));
    const auto found = store.find_product_by_id(p.id);
    REQUIRE(found.has_value());
    REQUIRE(found->current_stock == Approx(50.0));
}

// при поиске продукта по id=0, если продукт не найден, должно возвращаться nullopt
TEST_CASE_METHOD(InMemoryStoreTest, "FindProductByIdZeroIdNotFoundUnlessAssigned", "[InMemoryStoreTest]") {
    add_sample(store, "Z", 1.0);
    REQUIRE_FALSE(store.find_product_by_id(0).has_value());
}

// при поиске продукта по  отрицательному id, должно возвращаться nullopt
TEST_CASE_METHOD(InMemoryStoreTest, "FindProductByIdNegativeIdNotFound", "[InMemoryStoreTest]") {
    add_sample(store, "N", 1.0);
    REQUIRE_FALSE(store.find_product_by_id(-1).has_value());
}

// найти продукт по sku (7 тестов)

// при поиске продукта по sku, если продукт не найден, должно возвращаться nullopt
TEST_CASE_METHOD(InMemoryStoreTest, "FindProductBySkuReturnsNulloptWhenEmpty", "[InMemoryStoreTest]") {
    REQUIRE_FALSE(store.find_product_by_sku("ANY").has_value());
}

// при поиске продукта по sku, если продукт найден, должно возвращаться значение
TEST_CASE_METHOD(InMemoryStoreTest, "FindProductBySkuReturnsMatch", "[InMemoryStoreTest]") {
    add_sample(store, "SKU-100", 2.0);
    const auto found = store.find_product_by_sku("SKU-100");
    REQUIRE(found.has_value());
    REQUIRE(found->name == "Товар SKU-100");
}

// при поиске продукта по sku, если продукт найден, должно возвращаться значение и регистр должен быть чувствительным
TEST_CASE_METHOD(InMemoryStoreTest, "FindProductBySkuCaseSensitive", "[InMemoryStoreTest]") {
    add_sample(store, "AbC", 1.0);
    REQUIRE_FALSE(store.find_product_by_sku("abc").has_value());
    REQUIRE(store.find_product_by_sku("AbC").has_value());
}

// при поиске продукта по sku, если продукт не найден, должно возвращаться nullopt
TEST_CASE_METHOD(InMemoryStoreTest, "FindProductBySkuUnknownReturnsNullopt", "[InMemoryStoreTest]") {
    add_sample(store, "EXIST", 1.0);
    REQUIRE_FALSE(store.find_product_by_sku("MISSING").has_value());
}

// при поиске продукта по пустому sku, если товар с пустым sku сохранён, должно возвращаться значение
TEST_CASE_METHOD(InMemoryStoreTest, "FindProductBySkuEmptyStringNotFoundUnlessStored", "[InMemoryStoreTest]") {
    auto p = make_product("", "Пустой SKU", 0.0);
    store.add_product(p);
    REQUIRE(store.find_product_by_sku("").has_value());
}

// при поиске продукта по sku при дубликатах, должно возвращаться значение первого добавленного товара
TEST_CASE_METHOD(InMemoryStoreTest, "FindProductBySkuReturnsFirstWhenDuplicates", "[InMemoryStoreTest]") {
    const auto first = add_sample(store, "DUP", 1.0);
    add_sample(store, "DUP", 2.0);
    const auto found = store.find_product_by_sku("DUP");
    REQUIRE(found.has_value());
    REQUIRE(found->id == first.id);
}

// при переименовании sku через обновление, поиск по старому sku должен возвращать nullopt, по новому — значение
TEST_CASE_METHOD(InMemoryStoreTest, "FindProductBySkuAfterRenameViaUpdate", "[InMemoryStoreTest]") {
    auto p = add_sample(store, "OLD-SKU", 1.0);
    p.sku = "NEW-SKU";
    REQUIRE(store.update_product(p));
    REQUIRE_FALSE(store.find_product_by_sku("OLD-SKU").has_value());
    REQUIRE(store.find_product_by_sku("NEW-SKU").has_value());
}

// все продукты (6 тестов)

// при запросе всех продуктов в пустом хранилище, список должен быть пустым
TEST_CASE_METHOD(InMemoryStoreTest, "AllProductsEmptyInitially", "[InMemoryStoreTest]") {
    REQUIRE(store.all_products().empty());
}

// при добавлении одного продукта, all_products должен содержать один элемент
TEST_CASE_METHOD(InMemoryStoreTest, "AllProductsReturnsSingleItem", "[InMemoryStoreTest]") {
    add_sample(store, "S1", 1.0);
    REQUIRE(store.all_products().size() == 1u);
}

// при изменении возвращённого списка all_products, хранилище не должно изменяться
TEST_CASE_METHOD(InMemoryStoreTest, "AllProductsReturnsCopyNotReference", "[InMemoryStoreTest]") {
    auto list = store.all_products();
    list.clear();
    REQUIRE(store.all_products().size() == 0u);
}

// при запросе всех продуктов, all_products должен содержать все добавленные
TEST_CASE_METHOD(InMemoryStoreTest, "AllProductsContainsAllAdded", "[InMemoryStoreTest]") {
    add_sample(store, "P1", 1.0);
    add_sample(store, "P2", 2.0);
    add_sample(store, "P3", 3.0);
    REQUIRE(store.all_products().size() == 3u);
}

// при обновлении остатка продукта, all_products должен отражать новое значение current_stock
TEST_CASE_METHOD(InMemoryStoreTest, "AllProductsReflectsStockAfterUpdate", "[InMemoryStoreTest]") {
    auto p = add_sample(store, "ST", 5.0, 1.0);
    p.current_stock = 9.0;
    store.update_product(p);
    const auto list = store.all_products();
    REQUIRE(list.size() == 1u);
    REQUIRE(list[0].current_stock == Approx(9.0));
}

// при запросе all_products, порядок элементов должен совпадать с порядком добавления
TEST_CASE_METHOD(InMemoryStoreTest, "AllProductsPreservesInsertionOrder", "[InMemoryStoreTest]") {
    const auto a = add_sample(store, "FIRST", 1.0);
    const auto b = add_sample(store, "SECOND", 1.0);
    const auto list = store.all_products();
    REQUIRE(list.size() == 2u);
    REQUIRE(list[0].id == a.id);
    REQUIRE(list[1].id == b.id);
}

// добавить продукт (7 тестов)

// при добавлении нескольких продуктов, id должны назначаться последовательно с шагом 1
TEST_CASE_METHOD(InMemoryStoreTest, "AddProductAssignsIncrementalIds", "[InMemoryStoreTest]") {
    const auto p1 = add_sample(store, "I1", 1.0);
    const auto p2 = add_sample(store, "I2", 1.0);
    REQUIRE(p2.id == p1.id + 1);
}

// при добавлении продукта, sku и name должны сохраняться
TEST_CASE_METHOD(InMemoryStoreTest, "AddProductPreservesSkuAndName", "[InMemoryStoreTest]") {
    auto raw = make_product("KEEP", "Имя", 4.0, 0.0);
    raw.name = "Сохранённое имя";
    const auto added = store.add_product(raw);
    REQUIRE(added.sku == "KEEP");
    REQUIRE(added.name == "Сохранённое имя");
}

// при добавлении продукта, min_stock и current_stock должны сохраняться
TEST_CASE_METHOD(InMemoryStoreTest, "AddProductPreservesMinAndCurrentStock", "[InMemoryStoreTest]") {
    auto raw = make_product("STK", "S", 12.5, 3.3);
    const auto added = store.add_product(raw);
    REQUIRE(added.min_stock == Approx(12.5));
    REQUIRE(added.current_stock == Approx(3.3));
}

// при добавлении продукта, флаг active должен сохраняться
TEST_CASE_METHOD(InMemoryStoreTest, "AddProductPreservesActiveFlag", "[InMemoryStoreTest]") {
    auto raw = make_product("OFF", "S", 1.0, 0.0, false);
    const auto added = store.add_product(raw);
    REQUIRE_FALSE(added.active);
}

// при добавлении продукта с заданным id, хранилище должно назначить свой id, игнорируя входной
TEST_CASE_METHOD(InMemoryStoreTest, "AddProductIgnoresIncomingId", "[InMemoryStoreTest]") {
    auto raw = make_product("ID", "S", 1.0);
    raw.id = 5000;
    const auto added = store.add_product(raw);
    REQUIRE(added.id != 5000);
    REQUIRE(added.id == 1);
}

// при добавлении продукта, возвращаемая копия должна совпадать с данными в хранилище по назначенному id
TEST_CASE_METHOD(InMemoryStoreTest, "AddProductReturnsStoredCopyWithAssignedId", "[InMemoryStoreTest]") {
    const auto added = add_sample(store, "RET", 2.0);
    const auto found = store.find_product_by_id(added.id);
    REQUIRE(found.has_value());
    REQUIRE(found->sku == added.sku);
}

// при добавлении продукта, поле unit (единица измерения) должно сохраняться
TEST_CASE_METHOD(InMemoryStoreTest, "AddProductUnitFieldPreserved", "[InMemoryStoreTest]") {
    auto raw = make_product("U", "S", 1.0);
    raw.unit = "л";
    const auto added = store.add_product(raw);
    REQUIRE(added.unit == "л");
}

// обновить продукт (7 тестов)

// при обновлении продукта, если продукт не найден, должно возвращаться false
TEST_CASE_METHOD(InMemoryStoreTest, "UpdateProductReturnsFalseForUnknownId", "[InMemoryStoreTest]") {
    auto p = make_product("X", "Y", 1.0);
    p.id = 404;
    REQUIRE_FALSE(store.update_product(p));
}

// при обновлении продукта, если продукт найден, должно возвращаться true
TEST_CASE_METHOD(InMemoryStoreTest, "UpdateProductReturnsTrueForExisting", "[InMemoryStoreTest]") {
    auto p = add_sample(store, "UP", 1.0);
    p.name = "Новое имя";
    REQUIRE(store.update_product(p));
}

// при обновлении продукта, name должен измениться в хранилище
TEST_CASE_METHOD(InMemoryStoreTest, "UpdateProductChangesNameInStore", "[InMemoryStoreTest]") {
    auto p = add_sample(store, "NM", 1.0);
    p.name = "Обновлено";
    store.update_product(p);
    const auto found = store.find_product_by_id(p.id);
    REQUIRE(found.has_value());
    REQUIRE(found->name == "Обновлено");
}

// при обновлении продукта, current_stock должен измениться в хранилище
TEST_CASE_METHOD(InMemoryStoreTest, "UpdateProductChangesCurrentStock", "[InMemoryStoreTest]") {
    auto p = add_sample(store, "CS", 10.0, 2.0);
    p.current_stock = 20.0;
    store.update_product(p);
    const auto found = store.find_product_by_id(p.id);
    REQUIRE(found.has_value());
    REQUIRE(found->current_stock == Approx(20.0));
}

// при обновлении продукта, флаг active должен измениться в хранилище
TEST_CASE_METHOD(InMemoryStoreTest, "UpdateProductCanDeactivate", "[InMemoryStoreTest]") {
    auto p = add_sample(store, "ACT", 1.0);
    p.active = false;
    store.update_product(p);
    const auto found = store.find_product_by_id(p.id);
    REQUIRE(found.has_value());
    REQUIRE_FALSE(found->active);
}

// при обновлении продукта, id не должен изменяться
TEST_CASE_METHOD(InMemoryStoreTest, "UpdateProductDoesNotChangeId", "[InMemoryStoreTest]") {
    const auto p = add_sample(store, "IDK", 1.0);
    auto copy = p;
    copy.sku = "CHANGED";
    store.update_product(copy);
    const auto found = store.find_product_by_id(p.id);
    REQUIRE(found.has_value());
    REQUIRE(found->id == p.id);
}

// при обновлении продукта, другие продукты не должны быть затронуты
TEST_CASE_METHOD(InMemoryStoreTest, "UpdateProductDoesNotAffectOtherProducts", "[InMemoryStoreTest]") {
    const auto p1 = add_sample(store, "A", 1.0);
    auto p2 = add_sample(store, "B", 1.0);
    p2.name = "B-new";
    store.update_product(p2);
    const auto unchanged = store.find_product_by_id(p1.id);
    REQUIRE(unchanged.has_value());
    REQUIRE(unchanged->name == "Товар A");
}

// добавить транзакцию (6 тестов)

// при добавлении транзакции, id должны назначаться последовательно с шагом 1
TEST_CASE_METHOD(InMemoryStoreTest, "AddTransactionAssignsIncrementalIds", "[InMemoryStoreTest]") {
    StockTransaction t1;
    t1.product_id = 1;
    t1.quantity = 1.0;
    const auto a = store.add_transaction(t1);
    StockTransaction t2;
    t2.product_id = 1;
    t2.quantity = 2.0;
    const auto b = store.add_transaction(t2);
    REQUIRE(b.id == a.id + 1);
}

// при добавлении транзакции, type и comment должны сохраняться
TEST_CASE_METHOD(InMemoryStoreTest, "AddTransactionPreservesTypeAndComment", "[InMemoryStoreTest]") {
    StockTransaction tx;
    tx.product_id = 5;
    tx.type = TransactionType::Expense;
    tx.quantity = 3.5;
    tx.comment = "списание";
    const auto saved = store.add_transaction(tx);
    REQUIRE(saved.type == TransactionType::Expense);
    REQUIRE(saved.comment == "списание");
    REQUIRE(saved.quantity == Approx(3.5));
}

// при добавлении транзакции, id не должен изменяться
TEST_CASE_METHOD(InMemoryStoreTest, "AddTransactionIgnoresIncomingId", "[InMemoryStoreTest]") {
    StockTransaction tx;
    tx.id = 999;
    tx.product_id = 1;
    const auto saved = store.add_transaction(tx);
    REQUIRE(saved.id == 1);
}

// при добавлении транзакции, product_id должен сохраняться
TEST_CASE_METHOD(InMemoryStoreTest, "AddTransactionStoresProductId", "[InMemoryStoreTest]") {
    StockTransaction tx;
    tx.product_id = 42;
    const auto saved = store.add_transaction(tx);
    REQUIRE(saved.product_id == 42);
}

// при добавлении транзакции для одного и того же product_id, должно быть несколько транзакций
TEST_CASE_METHOD(InMemoryStoreTest, "AddTransactionMultipleForSameProduct", "[InMemoryStoreTest]") {
    StockTransaction tx;
    tx.product_id = 7;
    store.add_transaction(tx);
    store.add_transaction(tx);
    REQUIRE(store.transactions_for_product(7).size() == 2u);
}

// при добавлении транзакции, timestamp должен сохраняться, если он установлен
TEST_CASE_METHOD(InMemoryStoreTest, "AddTransactionTimestampPreservedIfSet", "[InMemoryStoreTest]") {
    StockTransaction tx;
    const auto ts = std::chrono::system_clock::now();
    tx.at = ts;
    const auto saved = store.add_transaction(tx);
    REQUIRE(saved.at == ts);
}

// транзакции для продукта (7 тестов)

// при запросе транзакций для пустого product_id, должно возвращаться пустой список
TEST_CASE_METHOD(InMemoryStoreTest, "TransactionsForProductEmptyWhenNone", "[InMemoryStoreTest]") {
    REQUIRE(store.transactions_for_product(1).empty());
}

// при запросе транзакций для product_id, должны возвращаться только транзакции для этого product_id
TEST_CASE_METHOD(InMemoryStoreTest, "TransactionsForProductFiltersByProductId", "[InMemoryStoreTest]") {
    StockTransaction t1;
    t1.product_id = 1;
    StockTransaction t2;
    t2.product_id = 2;
    store.add_transaction(t1);
    store.add_transaction(t2);
    REQUIRE(store.transactions_for_product(1).size() == 1u);
}

// при запросе транзакций для product_id, должны возвращаться все транзакции для этого product_id
TEST_CASE_METHOD(InMemoryStoreTest, "TransactionsForProductReturnsAllForProduct", "[InMemoryStoreTest]") {
    StockTransaction tx;
    tx.product_id = 3;
    store.add_transaction(tx);
    store.add_transaction(tx);
    store.add_transaction(tx);
    REQUIRE(store.transactions_for_product(3).size() == 3u);
}

// при запросе транзакций для несуществующего product_id, должно возвращаться пустой список
TEST_CASE_METHOD(InMemoryStoreTest, "TransactionsForProductUnknownIdEmpty", "[InMemoryStoreTest]") {
    StockTransaction tx;
    tx.product_id = 1;
    store.add_transaction(tx);
    REQUIRE(store.transactions_for_product(100).empty());
}

// при запросе транзакций для product_id, порядок транзакций должен совпадать с порядком добавления
TEST_CASE_METHOD(InMemoryStoreTest, "TransactionsForProductPreservesOrder", "[InMemoryStoreTest]") {
    StockTransaction t1;
    t1.product_id = 5;
    t1.quantity = 1.0;
    StockTransaction t2;
    t2.product_id = 5;
    t2.quantity = 2.0;
    const auto a = store.add_transaction(t1);
    const auto b = store.add_transaction(t2);
    const auto list = store.transactions_for_product(5);
    REQUIRE(list.size() == 2u);
    REQUIRE(list[0].id == a.id);
    REQUIRE(list[1].id == b.id);
}

// при запросе транзакций для product_id, другие продукты не должны включаться
TEST_CASE_METHOD(InMemoryStoreTest, "TransactionsForProductDoesNotIncludeOtherProducts", "[InMemoryStoreTest]") {
    StockTransaction a;
    a.product_id = 10;
    StockTransaction b;
    b.product_id = 11;
    store.add_transaction(a);
    store.add_transaction(b);
    const auto list = store.transactions_for_product(10);
    REQUIRE(list.size() == 1u);
    REQUIRE(list[0].product_id == 10);
}

// при запросе транзакций для отрицательного product_id, должно возвращаться пустой список
TEST_CASE_METHOD(InMemoryStoreTest, "TransactionsForProductNegativeIdEmpty", "[InMemoryStoreTest]") {
    StockTransaction tx;
    tx.product_id = 1;
    store.add_transaction(tx);
    REQUIRE(store.transactions_for_product(-5).empty());
}

// все транзакции (5 тестов)

// при запросе всех транзакций в пустом хранилище, должно возвращаться пустой список
TEST_CASE_METHOD(InMemoryStoreTest, "AllTransactionsEmptyInitially", "[InMemoryStoreTest]") {
    REQUIRE(store.all_transactions().empty());
}

// при запросе всех транзакций, должны возвращаться все добавленные транзакции
TEST_CASE_METHOD(InMemoryStoreTest, "AllTransactionsReturnsAllAdded", "[InMemoryStoreTest]") {
    store.add_transaction(StockTransaction{});
    store.add_transaction(StockTransaction{});
    REQUIRE(store.all_transactions().size() == 2u);
}

// при запросе всех транзакций, возвращаемый список должен быть копией, а не ссылкой
TEST_CASE_METHOD(InMemoryStoreTest, "AllTransactionsIsCopy", "[InMemoryStoreTest]") {
    auto list = store.all_transactions();
    list.clear();
    REQUIRE(store.all_transactions().size() == 0u);
}

// при запросе всех транзакций, должны возвращаться транзакции для всех продуктов
TEST_CASE_METHOD(InMemoryStoreTest, "AllTransactionsMixedProducts", "[InMemoryStoreTest]") {
    StockTransaction t1;
    t1.product_id = 1;
    StockTransaction t2;
    t2.product_id = 2;
    store.add_transaction(t1);
    store.add_transaction(t2);
    REQUIRE(store.all_transactions().size() == 2u);
}

// при запросе всех транзакций, последняя транзакция должна быть последней в списке
TEST_CASE_METHOD(InMemoryStoreTest, "AllTransactionsReflectsLatestAdd", "[InMemoryStoreTest]") {
    StockTransaction tx;
    tx.comment = "last";
    store.add_transaction(tx);
    const auto list = store.all_transactions();
    REQUIRE(list.size() == 1u);
    REQUIRE(list.back().comment == "last");
}

// добавить заказ (6 тестов)

// при добавлении заказа, id должны назначаться последовательно с шагом 1
TEST_CASE_METHOD(InMemoryStoreTest, "AddOrderAssignsIncrementalIds", "[InMemoryStoreTest]") {
    ReplenishmentOrder o1;
    o1.status = "draft";
    const auto a = store.add_order(o1);
    ReplenishmentOrder o2;
    o2.status = "sent";
    const auto b = store.add_order(o2);
    REQUIRE(b.id == a.id + 1);
}

// при добавлении заказа, status и items должны сохраняться
TEST_CASE_METHOD(InMemoryStoreTest, "AddOrderPreservesStatusAndItems", "[InMemoryStoreTest]") {
    ReplenishmentOrder order;
    order.status = "draft";
    order.items.push_back({1, 5.0});
    const auto saved = store.add_order(order);
    REQUIRE(saved.status == "draft");
    REQUIRE(saved.items.size() == 1u);
    REQUIRE(saved.items[0].quantity == Approx(5.0));
}

// при добавлении заказа, id не должен изменяться
TEST_CASE_METHOD(InMemoryStoreTest, "AddOrderIgnoresIncomingId", "[InMemoryStoreTest]") {
    ReplenishmentOrder order;
    order.id = 777;
    const auto saved = store.add_order(order);
    REQUIRE(saved.id == 1);
}

// при добавлении заказа, items может быть пустым
TEST_CASE_METHOD(InMemoryStoreTest, "AddOrderEmptyItemsAllowed", "[InMemoryStoreTest]") {
    ReplenishmentOrder order;
    order.status = "empty";
    const auto saved = store.add_order(order);
    REQUIRE(saved.items.empty());
}

// при добавлении заказа, created_at должен сохраняться
TEST_CASE_METHOD(InMemoryStoreTest, "AddOrderPreservesCreatedAt", "[InMemoryStoreTest]") {
    ReplenishmentOrder order;
    const auto ts = std::chrono::system_clock::now();
    order.created_at = ts;
    const auto saved = store.add_order(order);
    REQUIRE(saved.created_at == ts);
}

// при добавлении заказа, items может содержать несколько позиций
TEST_CASE_METHOD(InMemoryStoreTest, "AddOrderMultipleItems", "[InMemoryStoreTest]") {
    ReplenishmentOrder order;
    order.items = {{1, 1.0}, {2, 2.0}, {3, 3.0}};
    const auto saved = store.add_order(order);
    REQUIRE(saved.items.size() == 3u);
}

// все заказы (5 тестов)

// при запросе всех заказов в пустом хранилище, должно возвращаться пустой список
TEST_CASE_METHOD(InMemoryStoreTest, "AllOrdersEmptyInitially", "[InMemoryStoreTest]") {
    REQUIRE(store.all_orders().empty());
}

// при запросе всех заказов, должны возвращаться все добавленные заказы
TEST_CASE_METHOD(InMemoryStoreTest, "AllOrdersReturnsAll", "[InMemoryStoreTest]") {
    store.add_order(ReplenishmentOrder{});
    store.add_order(ReplenishmentOrder{});
    REQUIRE(store.all_orders().size() == 2u);
}

// при запросе всех заказов, возвращаемый список должен быть копией, а не ссылкой
TEST_CASE_METHOD(InMemoryStoreTest, "AllOrdersIsCopy", "[InMemoryStoreTest]") {
    auto list = store.all_orders();
    list.clear();
    REQUIRE(store.all_orders().size() == 0u);
}

// при запросе всех заказов, порядок заказов должен совпадать с порядком добавления
TEST_CASE_METHOD(InMemoryStoreTest, "AllOrdersPreservesInsertionOrder", "[InMemoryStoreTest]") {
    ReplenishmentOrder o1;
    o1.status = "first";
    ReplenishmentOrder o2;
    o2.status = "second";
    const auto a = store.add_order(o1);
    const auto b = store.add_order(o2);
    const auto list = store.all_orders();
    REQUIRE(list.size() == 2u);
    REQUIRE(list[0].id == a.id);
    REQUIRE(list[1].id == b.id);
}

// при запросе всех заказов, последний заказ должен быть последним в списке
TEST_CASE_METHOD(InMemoryStoreTest, "AllOrdersReflectsLatestStatus", "[InMemoryStoreTest]") {
    ReplenishmentOrder order;
    order.status = "draft";
    store.add_order(order);
    const auto list = store.all_orders();
    REQUIRE(list.size() == 1u);
    REQUIRE(list[0].status == "draft");
}
