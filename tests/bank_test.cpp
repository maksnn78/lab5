#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Account.h"
#include "Transaction.h"

using ::testing::NiceMock;

class MockAccount : public Account {
public:
    MockAccount(int id, int balance) : Account(id, balance) {}
    MOCK_METHOD(void, Unlock, (), (override));
};

class MockTransaction : public Transaction {
public:
    MOCK_METHOD(void, SaveToDataBase,
                (Account& from, Account& to, int sum), (override));
};

// ---- Account ----

TEST(Account, GetBalance) {
    NiceMock<MockAccount> acc(1, 100);
    EXPECT_EQ(acc.Account::GetBalance(), 100);
}

TEST(Account, ChangeBalanceWithoutLock) {
    NiceMock<MockAccount> acc(0, 100);
    EXPECT_THROW(acc.Account::ChangeBalance(50), std::runtime_error);
}

TEST(Account, ChangeBalanceWithLock) {
    NiceMock<MockAccount> acc(0, 100);
    acc.Account::Lock();
    acc.Account::ChangeBalance(50);
    EXPECT_EQ(acc.Account::GetBalance(), 150);
}

TEST(Account, LockTwiceThrows) {
    NiceMock<MockAccount> acc(0, 100);
    acc.Account::Lock();
    EXPECT_THROW(acc.Account::Lock(), std::runtime_error);
}

TEST(Account, UnlockCallsMock) {
    NiceMock<MockAccount> acc(0, 100);
    EXPECT_CALL(acc, Unlock()).Times(1);
    acc.Unlock();
}

// ---- Transaction ----

TEST(Transaction, DefaultFee) {
    Transaction tr;
    EXPECT_EQ(tr.fee(), 1);
}

TEST(Transaction, SetFee) {
    Transaction tr;
    tr.set_fee(100);
    EXPECT_EQ(tr.fee(), 100);
}

TEST(Transaction, MakeThrowsOnSameAccount) {
    Account acc(0, 5000);
    Transaction tr;
    EXPECT_THROW(tr.Make(acc, acc, 1000), std::logic_error);
}

TEST(Transaction, MakeThrowsOnNegativeSum) {
    Account from(0, 5000);
    Account to(1, 5000);
    Transaction tr;
    EXPECT_THROW(tr.Make(from, to, -50), std::invalid_argument);
}

TEST(Transaction, MakeThrowsWhenSumTooSmall) {
    Account from(0, 5000);
    Account to(1, 5000);
    Transaction tr;
    // sum < 100 -> logic_error "too small"
    EXPECT_THROW(tr.Make(from, to, 50), std::logic_error);
}

TEST(Transaction, MakeReturnsFalseWhenFeeTooBig) {
    Account from(0, 5000);
    Account to(1, 5000);
    Transaction tr;
    tr.set_fee(60);
    // fee*2=120 > sum=100 -> returns false
    EXPECT_FALSE(tr.Make(from, to, 100));
}

TEST(Transaction, MakeSuccess) {
    Account from(0, 5000);
    Account to(1, 5000);
    Transaction tr;
    tr.set_fee(100);
    // Debit списывает с `to`: to должен иметь > sum+fee после Credit
    // Credit(to, 1000): to=6000; Debit(to, 1100): 6000>1100 -> to=4900
    // from не меняется в этой реализации
    EXPECT_TRUE(tr.Make(from, to, 1000));
    EXPECT_EQ(to.GetBalance(), 4900);
}

TEST(Transaction, SaveToDataBaseMock) {
    NiceMock<MockAccount> from(0, 5000);
    NiceMock<MockAccount> to(1, 5000);
    MockTransaction tr;
    EXPECT_CALL(tr, SaveToDataBase(testing::Ref(from),
                                   testing::Ref(to), 1000)).Times(1);
    tr.SaveToDataBase(from, to, 1000);
}
