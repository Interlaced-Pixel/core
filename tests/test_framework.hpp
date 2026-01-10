#pragma once

#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

namespace pixellib_test {

struct TestCase {
  std::string_view name;
  void (*fn)();
};

inline std::vector<TestCase> &registry() {
  static std::vector<TestCase> tests;
  return tests;
}

inline int &failure_count() {
  static int failures = 0;
  return failures;
}

inline void record_failure(std::string_view expr, std::string_view file, int line) {
  ++failure_count();
  std::cerr << file << ":" << line << ": CHECK failed: " << expr << "\n";
}

struct Registrar {
  Registrar(std::string_view name, void (*fn)()) { registry().push_back(TestCase{name, fn}); }
};

class Approx {
public:
  explicit Approx(double value, double eps = 1e-9) : value_(value), eps_(eps) {}
  friend bool operator==(const Approx &a, double b) { return std::fabs(a.value_ - b) <= a.eps_; }
  friend bool operator==(double b, const Approx &a) { return a == b; }
  friend bool operator!=(const Approx &a, double b) { return !(a == b); }
  friend bool operator!=(double b, const Approx &a) { return !(a == b); }

private:
  double value_;
  double eps_;
};

} // namespace pixellib_test

#define PL_TEST_CONCAT_INNER(a, b) a##b
#define PL_TEST_CONCAT(a, b) PL_TEST_CONCAT_INNER(a, b)

// Keep existing suite structure compiling: `TEST_SUITE("name") { ... }`
#define TEST_SUITE(name) namespace

#define TEST_CASE(name) PL_TEST_TEST_CASE_IMPL(name, __COUNTER__)
#define PL_TEST_TEST_CASE_IMPL(name, id)                                                        \
  static void PL_TEST_CONCAT(pl_test_fn_, id)();                                                 \
  static pixellib_test::Registrar PL_TEST_CONCAT(pl_test_reg_, id)(name, &PL_TEST_CONCAT(pl_test_fn_, id)); \
  static void PL_TEST_CONCAT(pl_test_fn_, id)()

#define CHECK(expr)                                                                             \
  do {                                                                                          \
    if (!(expr)) {                                                                              \
      pixellib_test::record_failure(#expr, __FILE__, __LINE__);                                 \
    }                                                                                           \
  } while (0)

#define CHECK_FALSE(expr) CHECK(!(expr))

#define REQUIRE(expr)                                                                           \
  do {                                                                                          \
    if (!(expr)) {                                                                              \
      pixellib_test::record_failure(#expr, __FILE__, __LINE__);                                 \
      throw std::runtime_error("REQUIRE failed");                                                \
    }                                                                                           \
  } while (0)

#define REQUIRE_FALSE(expr) REQUIRE(!(expr))

#define CHECK_NOTHROW(expr)                                                                     \
  do {                                                                                          \
    try {                                                                                       \
      (void)(expr);                                                                             \
    } catch (const std::exception &e) {                                                         \
      pixellib_test::record_failure(e.what(), __FILE__, __LINE__);                               \
    } catch (...) {                                                                             \
      pixellib_test::record_failure("unexpected throw", __FILE__, __LINE__);                    \
    }                                                                                           \
  } while (0)

#define CHECK_THROWS_AS(expr, exc_type)                                                          \
  do {                                                                                           \
    bool pl_threw_expected = false;                                                              \
    try {                                                                                        \
      (void)(expr);                                                                              \
    } catch (const exc_type &) {                                                                 \
      pl_threw_expected = true;                                                                  \
    } catch (...) {                                                                              \
      pixellib_test::record_failure("threw wrong exception type", __FILE__, __LINE__);           \
      pl_threw_expected = true;                                                                  \
    }                                                                                            \
    if (!pl_threw_expected) {                                                                    \
      pixellib_test::record_failure("did not throw " #exc_type, __FILE__, __LINE__);             \
    }                                                                                            \
  } while (0)

#define INFO(msg)                                                                                \
  do {                                                                                           \
    std::cerr << "[INFO] " << msg << "\n";                                                       \
  } while (0)

