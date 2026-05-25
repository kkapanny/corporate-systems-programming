#include "inventory/transaction.h"

#include <gtest/gtest.h>

using inventory::StockTransaction;
using inventory::TransactionType;

TEST(TransactionModel, DefaultTypeIsIncome) {
    const StockTransaction tx;
    EXPECT_EQ(tx.type, TransactionType::Income);
}

TEST(TransactionModel, DefaultQuantityIsZero) {
    const StockTransaction tx;
    EXPECT_DOUBLE_EQ(tx.quantity, 0.0);
    EXPECT_EQ(tx.product_id, 0);
    EXPECT_EQ(tx.id, 0);
}

TEST(TransactionModel, IncomeTransactionFields) {
    StockTransaction tx;
    tx.id = 10;
    tx.product_id = 3;
    tx.type = TransactionType::Income;
    tx.quantity = 25.0;
    tx.comment = "поставка";

    EXPECT_EQ(tx.id, 10);
    EXPECT_EQ(tx.product_id, 3);
    EXPECT_EQ(tx.type, TransactionType::Income);
    EXPECT_DOUBLE_EQ(tx.quantity, 25.0);
    EXPECT_EQ(tx.comment, "поставка");
}

TEST(TransactionModel, ExpenseTransactionType) {
    StockTransaction tx;
    tx.type = TransactionType::Expense;
    tx.quantity = 4.0;
    EXPECT_EQ(tx.type, TransactionType::Expense);
    EXPECT_DOUBLE_EQ(tx.quantity, 4.0);
}

TEST(TransactionModel, TransactionTypeEnumDistinct) {
    EXPECT_NE(TransactionType::Income, TransactionType::Expense);
}

TEST(TransactionModel, CommentCanBeEmpty) {
    StockTransaction tx;
    tx.comment = "";
    EXPECT_TRUE(tx.comment.empty());
}

TEST(TransactionModel, TimestampAssignment) {
    StockTransaction tx;
    const auto t = std::chrono::system_clock::now();
    tx.at = t;
    EXPECT_EQ(tx.at, t);
}
