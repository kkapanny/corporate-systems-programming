#include "inventory/order.h"

#include <gtest/gtest.h>

using inventory::OrderItem;
using inventory::ReplenishmentOrder;

TEST(OrderModel, OrderItemDefaultValues) {
    const OrderItem item;
    EXPECT_EQ(item.product_id, 0);
    EXPECT_DOUBLE_EQ(item.quantity, 0.0);
}

TEST(OrderModel, OrderItemStoresProductAndQuantity) {
    OrderItem item;
    item.product_id = 15;
    item.quantity = 7.5;
    EXPECT_EQ(item.product_id, 15);
    EXPECT_DOUBLE_EQ(item.quantity, 7.5);
}

TEST(OrderModel, ReplenishmentOrderDefaultStatusEmpty) {
    const ReplenishmentOrder order;
    EXPECT_EQ(order.id, 0);
    EXPECT_TRUE(order.status.empty());
    EXPECT_TRUE(order.items.empty());
}

TEST(OrderModel, ReplenishmentOrderAcceptsStatusAndItems) {
    ReplenishmentOrder order;
    order.id = 3;
    order.status = "draft";
    order.items.push_back(OrderItem{1, 4.0});
    order.items.push_back(OrderItem{2, 1.5});

    EXPECT_EQ(order.id, 3);
    EXPECT_EQ(order.status, "draft");
    ASSERT_EQ(order.items.size(), 2u);
    EXPECT_EQ(order.items[0].product_id, 1);
    EXPECT_DOUBLE_EQ(order.items[0].quantity, 4.0);
    EXPECT_EQ(order.items[1].product_id, 2);
    EXPECT_DOUBLE_EQ(order.items[1].quantity, 1.5);
}

TEST(OrderModel, ReplenishmentOrderItemsVectorGrows) {
    ReplenishmentOrder order;
    for (int i = 1; i <= 5; ++i) {
        order.items.push_back(OrderItem{static_cast<std::int64_t>(i), static_cast<double>(i)});
    }
    EXPECT_EQ(order.items.size(), 5u);
    EXPECT_EQ(order.items.back().product_id, 5);
}

TEST(OrderModel, OrderItemFractionalQuantity) {
    OrderItem item{99, 0.001};
    EXPECT_DOUBLE_EQ(item.quantity, 0.001);
}

TEST(OrderModel, ReplenishmentOrderCreatedAtCanBeSet) {
    ReplenishmentOrder order;
    const auto now = std::chrono::system_clock::now();
    order.created_at = now;
    EXPECT_EQ(order.created_at, now);
}
