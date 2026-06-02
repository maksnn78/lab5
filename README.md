# Лабораторная работа №5
## Аксентьев Максим ИУ8-22
## CI Status
![CI](https://github.com/maksnn78/lab5/actions/workflows/ci.yml/badge.svg)
## Подготовка репозитория
```bash
git clone https://github.com/tp-labs/lab05 tasks/lab05
git clone https://github.com/maksnn78/lab5.git
cp -r ../tasks/lab05/banking .
git add .
git commit -m "initial import: banking library from tp-labs"
```
```bash
[main (root-commit) 5de26e9] initial import: banking library from tp-labs
 5 files changed, 134 insertions(+)
 create mode 100644 banking/Account.cpp
 create mode 100644 banking/Account.h
 create mode 100644 banking/CMakeList.txt
 create mode 100644 banking/Transaction.cpp
 create mode 100644 banking/Transaction.h
```
```bash
git push -u origin main
```
```bash
Enumerating objects: 8, done.
Counting objects: 100% (8/8), done.
Delta compression using up to 4 threads
Compressing objects: 100% (6/6), done.
Writing objects: 100% (8/8), 1.63 KiB | 238.00 KiB/s, done.
Total 8 (delta 0), reused 0 (delta 0), pack-reused 0
To https://github.com/maksnn78/lab5.git
 * [new branch]      main -> main
branch 'main' set up to track 'origin/main'.
```
## CMakeLists.txt для библиотеки banking
```bash
nano banking/CMakeLists.txt
```
```cmake
cmake_minimum_required(VERSION 3.4)
 
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
project(lib)
 
add_library(banking STATIC Account.cpp Transaction.cpp)
```
## Подключение GoogleTest и GMock
```bash
mkdir third-party
git submodule add https://github.com/google/googletest third-party/gtest
```
```bash
Cloning into '/home/vboxuser/lab5/third-party/gtest'...
remote: Enumerating objects: 28666, done.
remote: Counting objects: 100% (80/80), done.
remote: Compressing objects: 100% (61/61), done.
remote: Total 28666 (delta 43), reused 19 (delta 19), pack-reused 28586 (from 3)
Receiving objects: 100% (28666/28666), 13.81 MiB | 16.17 MiB/s, done.
Resolving deltas: 100% (21290/21290), done.
```
```bash
git add .gitmodules third-party/gtest
git commit -m "add googletest submodule"
```
```bash
[main 3b4ae3e] add googletest submodule
 2 files changed, 4 insertions(+)
 create mode 100644 .gitmodules
 create mode 160000 third-party/gtest
```
```bash
git push origin main
```
```bash
Enumerating objects: 5, done.
Counting objects: 100% (5/5), done.
Delta compression using up to 4 threads
Compressing objects: 100% (3/3), done.
Writing objects: 100% (4/4), 435 bytes | 145.00 KiB/s, done.
Total 4 (delta 0), reused 0 (delta 0), pack-reused 0
To https://github.com/maksnn78/lab5.git
   5de26e9..3b4ae3e  main -> main
```
## Создание тестов с mock-объектами
```bash
mkdir tests
nano tests/bank_test.cpp
```
```cpp
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
TEST(Account, UnlockActual) {
    Account acc(0, 100);
    acc.Lock();
    acc.Unlock();
    EXPECT_NO_THROW(acc.Lock());
}

TEST(Transaction, MakeThrowsWhenFromLocked) {
    Account from(0, 5000);
    Account to(1, 5000);
    Transaction tr;
    from.Lock();
    EXPECT_THROW(tr.Make(from, to, 1000), std::runtime_error);
}

TEST(Transaction, MakeThrowsWhenToLocked) {
    Account from(0, 5000);
    Account to(1, 5000);
    Transaction tr;
    to.Lock();
    EXPECT_THROW(tr.Make(from, to, 1000), std::runtime_error);
}
```
## Корневой CMakeLists.txt с покрытием кода
```bash
nano CMakeLists.txt
```
```cmake
cmake_minimum_required(VERSION 3.4)
 
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
project(lab05)
 
include_directories(banking)
add_subdirectory(banking banking)
 
option(BUILD_TESTS "Build tests" OFF)
 
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(third-party/gtest)
  add_executable(check tests/bank_test.cpp)
  target_compile_options(check PRIVATE --coverage)
  target_link_libraries(check banking gtest_main gmock_main --coverage)
  add_test(NAME check COMMAND check)
endif()
```
## Сборка и запуск тестов локально
```bash
cmake -S . -B build -DBUILD_TESTS=ON
```
```bash
CMake Deprecation Warning at CMakeLists.txt:1 (cmake_minimum_required):
  Compatibility with CMake < 3.5 will be removed from a future version of
  CMake.

  Update the VERSION argument <min> value or use a ...<max> suffix to tell
  CMake that the project does not need compatibility with older versions.


-- The C compiler identification is GNU 13.3.0
-- The CXX compiler identification is GNU 13.3.0
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/bin/cc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
CMake Deprecation Warning at banking/CMakeLists.txt:1 (cmake_minimum_required):
  Compatibility with CMake < 3.5 will be removed from a future version of
  CMake.

  Update the VERSION argument <min> value or use a ...<max> suffix to tell
  CMake that the project does not need compatibility with older versions.


-- Performing Test CMAKE_HAVE_LIBC_PTHREAD
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD - Success
-- Found Threads: TRUE  
-- Configuring done (2.0s)
-- Generating done (0.0s)
-- Build files have been written to: /home/vboxuser/lab5/build
```
```bash
cmake --build build
```
```bash
[  7%] Building CXX object third-party/gtest/googletest/CMakeFiles/gtest.dir/src/gtest-all.cc.o
[ 15%] Linking CXX static library ../../../lib/libgtest.a
[ 15%] Built target gtest
[ 23%] Building CXX object third-party/gtest/googletest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.o
[ 30%] Linking CXX static library ../../../lib/libgtest_main.a
[ 30%] Built target gtest_main
[ 38%] Building CXX object banking/CMakeFiles/banking.dir/Account.cpp.o
[ 46%] Building CXX object banking/CMakeFiles/banking.dir/Transaction.cpp.o
[ 53%] Linking CXX static library libbanking.a
[ 53%] Built target banking
[ 61%] Building CXX object third-party/gtest/googlemock/CMakeFiles/gmock.dir/src/gmock-all.cc.o
[ 69%] Linking CXX static library ../../../lib/libgmock.a
[ 69%] Built target gmock
[ 76%] Building CXX object third-party/gtest/googlemock/CMakeFiles/gmock_main.dir/src/gmock_main.cc.o
[ 84%] Linking CXX static library ../../../lib/libgmock_main.a
[ 84%] Built target gmock_main
[ 92%] Building CXX object CMakeFiles/check.dir/tests/bank_test.cpp.o
[100%] Linking CXX executable check
[100%] Built target check
```
## Проверка покрытия кода
```bash
cd build
./check
```
```bash
Running main() from /home/vboxuser/lab5/third-party/gtest/googletest/src/gtest_main.cc
[==========] Running 13 tests from 2 test suites.
[----------] Global test environment set-up.
[----------] 5 tests from Account
[ RUN      ] Account.GetBalance
[       OK ] Account.GetBalance (0 ms)
[ RUN      ] Account.ChangeBalanceWithoutLock
[       OK ] Account.ChangeBalanceWithoutLock (0 ms)
[ RUN      ] Account.ChangeBalanceWithLock
[       OK ] Account.ChangeBalanceWithLock (0 ms)
[ RUN      ] Account.LockTwiceThrows
[       OK ] Account.LockTwiceThrows (0 ms)
[ RUN      ] Account.UnlockCallsMock
[       OK ] Account.UnlockCallsMock (0 ms)
[----------] 5 tests from Account (1 ms total)

[----------] 8 tests from Transaction
[ RUN      ] Transaction.DefaultFee
[       OK ] Transaction.DefaultFee (0 ms)
[ RUN      ] Transaction.SetFee
[       OK ] Transaction.SetFee (0 ms)
[ RUN      ] Transaction.MakeThrowsOnSameAccount
[       OK ] Transaction.MakeThrowsOnSameAccount (0 ms)
[ RUN      ] Transaction.MakeThrowsOnNegativeSum
[       OK ] Transaction.MakeThrowsOnNegativeSum (0 ms)
[ RUN      ] Transaction.MakeThrowsWhenSumTooSmall
[       OK ] Transaction.MakeThrowsWhenSumTooSmall (0 ms)
[ RUN      ] Transaction.MakeReturnsFalseWhenFeeTooBig
[       OK ] Transaction.MakeReturnsFalseWhenFeeTooBig (0 ms)
[ RUN      ] Transaction.MakeSuccess
0 send to 1 $1000
Balance 0 is 5000
Balance 1 is 4900
[       OK ] Transaction.MakeSuccess (0 ms)
[ RUN      ] Transaction.SaveToDataBaseMock
[       OK ] Transaction.SaveToDataBaseMock (0 ms)
[----------] 8 tests from Transaction (0 ms total)

[----------] Global test environment tear-down
[==========] 13 tests from 2 test suites ran. (2 ms total)
[  PASSED  ] 13 tests.
```
```bash
lcov --capture --directory . --output-file coverage.info \
     --ignore-errors mismatch,inconsistent,gcov
lcov --remove coverage.info '/usr/*' '*/third-party/*' '*/tests/*' \
     --output-file coverage.info \
     --ignore-errors empty,inconsistent,unused
genhtml coverage.info --output-directory coverage_html \
        --ignore-errors inconsistent
lcov --list coverage.info --ignore-errors inconsistent
```
```bash
                   |Lines       |Functions  |Branches    
Filename           |Rate     Num|Rate    Num|Rate     Num
=========================================================
[/home/vboxuser/lab5/banking/]
Transaction.h      | 100%      2| 0.0%     2|    -      0
=========================================================
             Total:| 100%      2| 0.0%     2|    -      0
```
## Настройка GitHub Actions
```bash
mkdir -p .github/workflows
nano .github/workflows/ci.yml
```
```yaml
name: CI
 
on:
  push:
  pull_request:
 
jobs:
  build-and-test:
    runs-on: ubuntu-latest
 
    steps:
      - uses: actions/checkout@v4
        with:
          submodules: recursive
 
      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y cmake g++ lcov
          pip install cpp-coveralls
 
      - name: Configure project
        run: cmake -S . -B build -DBUILD_TESTS=ON
 
      - name: Build project
        run: cmake --build build
 
      - name: Run tests
        run: ctest --test-dir build --output-on-failure
 
      - name: Collect coverage
        run: |
          cd build
          lcov --capture --directory . --output-file coverage.info
          lcov --remove coverage.info '/usr/*' '*/third-party/*' '*/tests/*' \
               --output-file coverage.info
          lcov --list coverage.info
 
      - name: Upload coverage to Coveralls
        env:
          COVERALLS_REPO_TOKEN: ${{ secrets.COVERALLS_REPO_TOKEN }}
        run: |
          coveralls --root . -E ".*gtest.*" -E ".*CMakeFiles.*" \
                    -E ".*third-party.*"
```
## Коммиты и пуш
```bash
git add banking/CMakeLists.txt CMakeLists.txt tests/bank_test.cpp \
        .github/workflows/ci.yml
git commit -m "add banking CMakeLists, unit tests with mocks, CI and coverage"
```
```bash
[main ffd0211] add banking CMakeLists, unit tests with mocks, CI and coverage
 4 files changed, 185 insertions(+)
 create mode 100644 .github/workflows/ci.yml
 create mode 100644 CMakeLists.txt
 create mode 100644 banking/CMakeLists.txt
 create mode 100644 tests/bank_test.cpp
```
```bash
git push origin main
```
```bash
Enumerating objects: 12, done.
Counting objects: 100% (12/12), done.
Delta compression using up to 4 threads
Compressing objects: 100% (7/7), done.
Writing objects: 100% (10/10), 2.47 KiB | 281.00 KiB/s, done.
Total 10 (delta 1), reused 0 (delta 0), pack-reused 0
remote: Resolving deltas: 100% (1/1), completed with 1 local object.
To https://github.com/maksnn78/lab5.git
   3b4ae3e..ffd0211  main -> main
```
