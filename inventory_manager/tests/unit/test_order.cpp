#include "inventory/order.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using Catch::Approx;

using inventory::OrderItem;
using inventory::ReplenishmentOrder;

// при создании заказа, продукт и количество должны быть равны 0
TEST_CASE("OrderItemDefaultValues", "[OrderModel]") {
    const OrderItem item;
    REQUIRE(item.product_id == 0);
    REQUIRE(item.quantity == Approx(0.0));
}

// при присвоении значений полям заказа, они должны сохраняться
TEST_CASE("OrderItemStoresProductAndQuantity", "[OrderModel]") {
    OrderItem item;
    item.product_id = 15;
    item.quantity = 7.5;
    REQUIRE(item.product_id == 15);
    REQUIRE(item.quantity == Approx(7.5));
}

// при создании заказа, статус должен быть пустой строкой
TEST_CASE("ReplenishmentOrderDefaultStatusEmpty", "[OrderModel]") {
    const ReplenishmentOrder order;
    REQUIRE(order.id == 0);
    REQUIRE(order.status.empty());
    REQUIRE(order.items.empty());
}

// при присвоении значений полям заказа, они должны сохраняться
TEST_CASE("ReplenishmentOrderAcceptsStatusAndItems", "[OrderModel]") {
    ReplenishmentOrder order;
    order.id = 3;
    order.status = "draft";
    order.items.push_back(OrderItem{1, 4.0});
    order.items.push_back(OrderItem{2, 1.5});

    REQUIRE(order.id == 3);
    REQUIRE(order.status == "draft");
    REQUIRE(order.items.size() == 2u);
    REQUIRE(order.items[0].product_id == 1);
    REQUIRE(order.items[0].quantity == Approx(4.0));
    REQUIRE(order.items[1].product_id == 2);
    REQUIRE(order.items[1].quantity == Approx(1.5));
}

// при добавлении заказа, количество позиций должно увеличиваться
TEST_CASE("ReplenishmentOrderItemsVectorGrows", "[OrderModel]") {
    ReplenishmentOrder order;
    for (int i = 1; i <= 5; ++i) {
        order.items.push_back(OrderItem{static_cast<std::int64_t>(i), static_cast<double>(i)});
    }
    REQUIRE(order.items.size() == 5u);
    REQUIRE(order.items.back().product_id == 5);
}

// при создании заказа, дробное количество должно быть сохранено
TEST_CASE("OrderItemFractionalQuantity", "[OrderModel]") {
    OrderItem item{99, 0.001};
    REQUIRE(item.quantity == Approx(0.001));
}

// при создании заказа, время создания должно быть сохранено
TEST_CASE("ReplenishmentOrderCreatedAtCanBeSet", "[OrderModel]") {
    ReplenishmentOrder order;
    const auto now = std::chrono::system_clock::now();
    order.created_at = now;
    REQUIRE(order.created_at == now);
}
