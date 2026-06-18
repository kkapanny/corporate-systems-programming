#include "inventory/transaction.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using Catch::Approx;

using inventory::StockTransaction;
using inventory::TransactionType;

// при создании транзакции, тип должен быть Income (приход)
TEST_CASE("DefaultTypeIsIncome", "[TransactionModel]") {
    const StockTransaction tx;
    REQUIRE(tx.type == TransactionType::Income);
}

// при создании транзакции, количество, продукт и id должны быть равны 0
TEST_CASE("DefaultQuantityIsZero", "[TransactionModel]") {
    const StockTransaction tx;
    REQUIRE(tx.quantity == Approx(0.0));
    REQUIRE(tx.product_id == 0);
    REQUIRE(tx.id == 0);
}

// при присвоении значений полям транзакции, они должны сохраняться
TEST_CASE("IncomeTransactionFields", "[TransactionModel]") {
    StockTransaction tx;
    tx.id = 10;
    tx.product_id = 3;
    tx.type = TransactionType::Income;
    tx.quantity = 25.0;
    tx.comment = "поставка";

    REQUIRE(tx.id == 10);
    REQUIRE(tx.product_id == 3);
    REQUIRE(tx.type == TransactionType::Income);
    REQUIRE(tx.quantity == Approx(25.0));
    REQUIRE(tx.comment == "поставка");
}

// при присвоении значений полям транзакции, тип должен быть Expense (расход) и количество должно быть равно 4.0
TEST_CASE("ExpenseTransactionType", "[TransactionModel]") {
    StockTransaction tx;
    tx.type = TransactionType::Expense;
    tx.quantity = 4.0;
    REQUIRE(tx.type == TransactionType::Expense);
    REQUIRE(tx.quantity == Approx(4.0));
}

// при создании транзакции, типы Income и Expense должны быть различными
TEST_CASE("TransactionTypeEnumDistinct", "[TransactionModel]") {
    REQUIRE(TransactionType::Income != TransactionType::Expense);
}

// при создании транзакции, комментарий должен быть пустой строкой
TEST_CASE("CommentCanBeEmpty", "[TransactionModel]") {
    StockTransaction tx;
    tx.comment = "";
    REQUIRE(tx.comment.empty());
}

// при присвоении значению поля времени транзакции, оно должно сохраняться
TEST_CASE("TimestampAssignment", "[TransactionModel]") {
    StockTransaction tx;
    const auto t = std::chrono::system_clock::now();
    tx.at = t;
    REQUIRE(tx.at == t);
}
