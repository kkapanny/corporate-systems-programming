#include "inventory/product.h"

#include <gtest/gtest.h>

using inventory::Product;

TEST(ProductModel, DefaultConstructionHasZeroIdAndEmptyStrings) {
    const Product p;
    EXPECT_EQ(p.id, 0);
    EXPECT_TRUE(p.sku.empty());
    EXPECT_TRUE(p.name.empty());
    EXPECT_TRUE(p.unit.empty());
}

TEST(ProductModel, DefaultStockValuesAreZero) {
    const Product p;
    EXPECT_DOUBLE_EQ(p.min_stock, 0.0);
    EXPECT_DOUBLE_EQ(p.current_stock, 0.0);
}

TEST(ProductModel, DefaultActiveFlagIsTrue) {
    const Product p;
    EXPECT_TRUE(p.active);
}

TEST(ProductModel, FieldAssignmentPersistsValues) {
    Product p;
    p.id = 42;
    p.sku = "SKU-001";
    p.name = "Болты";
    p.unit = "кг";
    p.min_stock = 10.5;
    p.current_stock = 3.25;
    p.active = false;

    EXPECT_EQ(p.id, 42);
    EXPECT_EQ(p.sku, "SKU-001");
    EXPECT_EQ(p.name, "Болты");
    EXPECT_EQ(p.unit, "кг");
    EXPECT_DOUBLE_EQ(p.min_stock, 10.5);
    EXPECT_DOUBLE_EQ(p.current_stock, 3.25);
    EXPECT_FALSE(p.active);
}

TEST(ProductModel, CopyPreservesAllFields) {
    Product original;
    original.id = 7;
    original.sku = "A-7";
    original.name = "Гайки";
    original.unit = "шт";
    original.min_stock = 5.0;
    original.current_stock = 12.0;
    original.active = true;

    const Product copy = original;
    EXPECT_EQ(copy.id, original.id);
    EXPECT_EQ(copy.sku, original.sku);
    EXPECT_EQ(copy.name, original.name);
    EXPECT_EQ(copy.unit, original.unit);
    EXPECT_DOUBLE_EQ(copy.min_stock, original.min_stock);
    EXPECT_DOUBLE_EQ(copy.current_stock, original.current_stock);
    EXPECT_EQ(copy.active, original.active);
}

TEST(ProductModel, CurrentStockCanExceedMinStock) {
    Product p;
    p.min_stock = 5.0;
    p.current_stock = 100.0;
    EXPECT_GT(p.current_stock, p.min_stock);
}

TEST(ProductModel, InactiveProductStillHoldsStockFields) {
    Product p;
    p.active = false;
    p.current_stock = 8.0;
    p.min_stock = 20.0;
    EXPECT_FALSE(p.active);
    EXPECT_LT(p.current_stock, p.min_stock);
}
