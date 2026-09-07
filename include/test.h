// =============================================================================
//  test.h —— 零依赖的极简单元测试框架
//
//  用法：
//      TEST(hash, q1_two_sum) {
//        lc0001::Solution s;
//        vector<int> a{2, 7, 11, 15};
//        CHECK_EQ(s.twoSum(a, 9), vector<int>{0, 1});
//      }
//
//  每个 TEST 会自动注册，main.cpp 统一跑；命令行可传子串过滤：
//      ./hot100 hash        # 只跑哈希那一组
//      ./hot100 q1_         # 只跑跑名字里含 q1_ 的用例
// =============================================================================
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

#include "common.h"

namespace lc {

struct Failure {
  std::string msg;
};

struct Case {
  std::string suite;
  std::string name;
  std::function<void()> body;
  std::string id() const { return suite + "/" + name; }
};

inline std::vector<Case>& cases() {
  static std::vector<Case> c;
  return c;
}

struct Registrar {
  Registrar(const char* suite, const char* name, std::function<void()> fn) {
    cases().push_back({suite, name, std::move(fn)});
  }
};

[[noreturn]] inline void fail(const char* file, int line, const std::string& msg) {
  throw Failure{std::string(file) + ":" + std::to_string(line) + ": " + msg};
}

template <class A, class B>
void checkEqImpl(const A& actual, const B& expect, const char* file, int line,
                 const char* exprs) {
  if (!(actual == expect)) {
    fail(file, line,
         std::string("CHECK_EQ(") + exprs +
             ")\n      actual   = " + toStr(actual) + "\n      expected = " + toStr(expect));
  }
}

inline void checkTrue(bool cond, const char* file, int line, const char* expr) {
  if (!cond) fail(file, line, std::string("CHECK_TRUE(") + expr + ")");
}

// 浮点比较（Q4 中位数、Q295 中位数会用到）
template <class A, class B>
void checkNearImpl(const A& actual, const B& expect, double eps, const char* file, int line,
                   const char* exprs) {
  double diff = std::abs(static_cast<double>(actual) - static_cast<double>(expect));
  if (diff > eps) {
    fail(file, line, std::string("CHECK_NEAR(") + exprs + ")  |diff|=" +
                         std::to_string(diff) + " > " + std::to_string(eps) +
                         "\n      actual   = " + toStr(actual) +
                         "\n      expected = " + toStr(expect));
  }
}

// filter 为空表示全跑；否则按 "suite/name" 子串匹配
inline int runAll(const std::string& filter = "") {
  int total = 0, passed = 0;
  std::string curSuite;
  for (const Case& c : cases()) {
    std::string id = c.id();
    if (!filter.empty() && id.find(filter) == std::string::npos) continue;
    if (c.suite != curSuite) {
      if (!curSuite.empty()) std::printf("\n");
      curSuite = c.suite;
      std::printf("[%s]\n    ", curSuite.c_str());
    }
    ++total;
    try {
      c.body();
      std::printf(".");
      ++passed;
    } catch (const Failure& f) {
      std::printf("F\n");
      std::printf("  \033[31mFAILED\033[0m %s\n  %s\n    ", id.c_str(), f.msg.c_str());
    } catch (const std::exception& e) {
      std::printf("E\n");
      std::printf("  \033[31mERROR \033[0m %s\n  threw: %s\n    ", id.c_str(), e.what());
    } catch (...) {
      std::printf("E\n");
      std::printf("  \033[31mERROR \033[0m %s\n  threw: unknown exception\n    ", id.c_str());
    }
  }
  std::printf("\n\n%d/%d test cases passed.\n", passed, total);
  if (passed != total)
    std::printf("\033[31m=> 有用例失败，见上方 FAILED\033[0m\n");
  else if (total)
    std::printf("\033[32m=> 全部通过\033[0m\n");
  else
    std::printf("\033[33m=> 过滤条件没匹配到任何用例（拼错了？）\033[0m\n");
  return (total > 0 && passed == total) ? 0 : 1;  // 没匹配到任何用例也算失败（过滤条件写错了）
}

}  // namespace lc

#define TEST(suite, name)                                                     \
  static void suite##_##name##_body();                                        \
  static ::lc::Registrar suite##_##name##_reg(#suite, #name,                  \
                                              &suite##_##name##_body);        \
  static void suite##_##name##_body()

// 注意：宏故意做成变参，这样 CHECK_EQ(x, vector<int>{0, 1}) 里的花括号逗号
// 不会被预处理器误当作参数分隔符（写用例时不用手动加括号）。
#define CHECK_EQ(...) ::lc::checkEqImpl(__VA_ARGS__, __FILE__, __LINE__, #__VA_ARGS__)
#define CHECK_TRUE(...) ::lc::checkTrue(!!(__VA_ARGS__), __FILE__, __LINE__, #__VA_ARGS__)
#define CHECK_FALSE(...) ::lc::checkTrue(!(__VA_ARGS__), __FILE__, __LINE__, "!(" #__VA_ARGS__ ")")
#define CHECK_NEAR(...) ::lc::checkNearImpl(__VA_ARGS__, __FILE__, __LINE__, #__VA_ARGS__)
