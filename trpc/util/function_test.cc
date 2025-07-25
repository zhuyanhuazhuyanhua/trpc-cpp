//
//
// Copyright (C) 2018 Tencent. All rights reserved.
// Flare is licensed under the BSD 3-Clause License.
// The source codes in this file based on
// https://github.com/Tencent/flare/blob/master/flare/base/function_test.cc.
// This source file may have been modified by Tencent, and licensed under the BSD 3-Clause License.
//
//

#include "trpc/util/function.h"

#include <array>
#include <vector>

#include "gtest/gtest.h"

namespace trpc::testing {

int PlainOldFunction(int, double, char) { return 1; }

TEST(Function, Empty) { Function<void()> f; }

TEST(Function, POF) {
  Function<int(int, double, char)> f(PlainOldFunction);
  ASSERT_EQ(1, f(0, 0, 0));

#if __cpp_deduction_guides >= 201611L
  Function f2(PlainOldFunction);  // Deduction guides come into play.
  ASSERT_EQ(1, f2(0, 0, 0));
#endif
}

TEST(Function, Lambda) {
#if __cpp_deduction_guides >= 201611L
  Function f([](int x) { return x; });
  ASSERT_EQ(12, f(12));
#endif

  Function<int()> f2([] { return 1; });
  ASSERT_EQ(1, f2());
}

class FancyClass {
 public:
  int f(int x) { return x; }
};

TEST(Function, MemberMethod) {
  Function<int(FancyClass&, int)> f(&FancyClass::f);
  FancyClass fc;

  ASSERT_EQ(10, f(fc, 10));
}

TEST(Function, LargeFunctorTest) {
  Function<int()> f;
  std::array<char, 1000000> payload;

  payload.back() = 12;
  f = [payload] { return payload.back(); };
  ASSERT_EQ(12, f());
}

TEST(Function, FunctorMoveTest) {
  struct OnlyCopyable {
    OnlyCopyable() : v(new std::vector<int>()) {}
    OnlyCopyable(const OnlyCopyable& oc) : v(new std::vector<int>(*oc.v)) {}
    ~OnlyCopyable() { delete v; }
    std::vector<int>* v;
  };
  Function<int()> f, f2;
  OnlyCopyable payload;

  payload.v->resize(100, 12);

  // BE SURE THAT THE LAMBDA IS NOT LARGER THAN kMaximumOptimizableSize.
  f = [payload] { return payload.v->back(); };
  f2 = std::move(f);
  ASSERT_EQ(12, f2());
}

TEST(Function, LargeFunctorMoveTest) {
  Function<int()> f, f2;
  std::array<std::vector<int>, 100> payload;

  payload.back().resize(10, 12);
  f = [payload] { return payload.back().back(); };
  f2 = std::move(f);
  ASSERT_EQ(12, f2());
}

TEST(Function, CastAnyTypeToVoid) {
  Function<void()> f;
  int x = 0;

  f = [&x]() -> int {
    x = 1;
    return x;
  };
  f();

  ASSERT_EQ(1, x);
}

TEST(Function, Clear) {
  Function<void()> f = [] {};

  ASSERT_TRUE(f);
  f = nullptr;
  ASSERT_FALSE(f);

  std::function<void()> f1 = nullptr;
  Function<void()> f2 = f1;
  ASSERT_FALSE(f2);
  f = std::move(f1);
  ASSERT_FALSE(f);
  std::function<void()> f3 = []() {};
  f = std::move(f3);
  ASSERT_TRUE(f);

  Function<void()> f4([]() {});
  ASSERT_TRUE(f4);
}

}  // namespace trpc::testing
