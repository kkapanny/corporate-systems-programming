#include "inventory/product.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using Catch::Approx;

using inventory::Product;

// при создании товара по умолчанию, id должен быть 0, а строковые поля — пустыми
TEST_CASE("DefaultConstructionHasZeroIdAndEmptyStrings", "[ProductModel]") {
    const Product p;
    REQUIRE(p.id == 0);
    REQUIRE(p.sku.empty());
    REQUIRE(p.name.empty());
    REQUIRE(p.unit.empty());
}

// при создании товара, минимальный остаток и текущий остаток должны быть равны 0
TEST_CASE("DefaultStockValuesAreZero", "[ProductModel]") {
    const Product p;
    REQUIRE(p.min_stock == Approx(0.0));
    REQUIRE(p.current_stock == Approx(0.0));
}

// при создании товара, активный флаг должен быть true
TEST_CASE("DefaultActiveFlagIsTrue", "[ProductModel]") {
    const Product p;
    REQUIRE(p.active);
}

// при присвоении значений полям товара, они должны сохраняться
TEST_CASE("FieldAssignmentPersistsValues", "[ProductModel]") {
    Product p;
    p.id = 42;
    p.sku = "SKU-001";
    p.name = "Болты";
    p.unit = "кг";
    p.min_stock = 10.5;
    p.current_stock = 3.25;
    p.active = false;

    REQUIRE(p.id == 42);
    REQUIRE(p.sku == "SKU-001");
    REQUIRE(p.name == "Болты");
    REQUIRE(p.unit == "кг");
    REQUIRE(p.min_stock == Approx(10.5));
    REQUIRE(p.current_stock == Approx(3.25));
    REQUIRE_FALSE(p.active);
}

// при копировании товара, все поля должны сохраняться
TEST_CASE("CopyPreservesAllFields", "[ProductModel]") {
    Product original;
    original.id = 7;
    original.sku = "A-7";
    original.name = "Гайки";
    original.unit = "шт";
    original.min_stock = 5.0;
    original.current_stock = 12.0;
    original.active = true;

    const Product copy = original;
    REQUIRE(copy.id == original.id);
    REQUIRE(copy.sku == original.sku);
    REQUIRE(copy.name == original.name);
    REQUIRE(copy.unit == original.unit);
    REQUIRE(copy.min_stock == Approx(original.min_stock));
    REQUIRE(copy.current_stock == Approx(original.current_stock));
    REQUIRE(copy.active == original.active);
}

// при текущем остатке больше минимального, текущий остаток должен быть больше минимального
TEST_CASE("CurrentStockCanExceedMinStock", "[ProductModel]") {
    Product p;
    p.min_stock = 5.0;
    p.current_stock = 100.0;
    REQUIRE(p.current_stock > p.min_stock);
}

// при активном флаге false, текущий остаток должен быть меньше минимального
TEST_CASE("InactiveProductStillHoldsStockFields", "[ProductModel]") {
    Product p;
    p.active = false;
    p.current_stock = 8.0;
    p.min_stock = 20.0;
    REQUIRE_FALSE(p.active);
    REQUIRE(p.current_stock < p.min_stock);
}
