#pragma once

#include <cstddef>
#include <cstdint>
#include <source_location>

#define _KMERGE(x, y) x##y
#define _KUNIQUEID_IMPL(prefix, suffix) _KMERGE(prefix, suffix)
#define _KUNIQUEID(prefix) _KUNIQUEID_IMPL(prefix, __LINE__)

#ifdef BUILD_TESTS
namespace ktest {
class KTest {
 public:
  KTest(const char* name);

  bool run();

 protected:
  virtual void do_run() = 0;
  bool __ktest_current_ok = true;

  bool __ktest_expect(bool cond, std::source_location location = std::source_location::current());

 private:
  const char* __ktest_current_name;
};  // class KTest

void run_tests();
}  // namespace ktest

#define TEST(name)                                                                    \
  class _KUNIQUEID(_KTest) : public ::ktest::KTest {                                  \
   public:                                                                            \
    using KTest::KTest;                                                               \
                                                                                      \
   private:                                                                           \
    void do_run() override;                                                           \
  };                                                                                  \
  static _KUNIQUEID(_KTest) _KUNIQUEID(__ktest_instance_) = _KUNIQUEID(_KTest)(name); \
  void _KUNIQUEID(_KTest)::do_run()

#define _KTEST_EXPECT_IMPL(cond) __ktest_expect(!!(cond));
#define _KTEST_ASSERT_IMPL(macro) \
  if (macro)                      \
    return;
#else
namespace ktest {
static inline void run_tests() {}
}  // namespace ktest

#define TEST(name) void _KUNIQUEID(__ktest_unused_)()
#define _KTEST_EXPECT_IMPL(cond)
#define _KTEST_ASSERT_IMPL(macro)
#endif  // BUILD_TESTS

#define EXPECT_TRUE(a) _KTEST_EXPECT_IMPL((a))
#define ASSERT_TRUE(a) _KTEST_ASSERT_IMPL(EXPECT_TRUE(a))

#define EXPECT_FALSE(a) _KTEST_EXPECT_IMPL(!(a))
#define ASSERT_FALSE(a) _KTEST_ASSERT_IMPL(EXPECT_FALSE(a))

#define EXPECT_EQ(a, b) _KTEST_EXPECT_IMPL((a) == (b))
#define ASSERT_EQ(a, b) _KTEST_ASSERT_IMPL(EXPECT_EQ(a, b))

#define EXPECT_NE(a, b) _KTEST_EXPECT_IMPL((a) != (b))
#define ASSERT_NE(a, b) _KTEST_ASSERT_IMPL(EXPECT_NE(a, b))

#define EXPECT_LT(a, b) _KTEST_EXPECT_IMPL((a) < (b))
#define ASSERT_LT(a, b) _KTEST_ASSERT_IMPL(EXPECT_LT(a, b))

#define EXPECT_LE(a, b) _KTEST_EXPECT_IMPL((a) <= (b))
#define ASSERT_LE(a, b) _KTEST_ASSERT_IMPL(EXPECT_LE(a, b))

#define EXPECT_GT(a, b) _KTEST_EXPECT_IMPL((a) > (b))
#define ASSERT_GT(a, b) _KTEST_ASSERT_IMPL(EXPECT_GT(a, b))

#define EXPECT_GE(a, b) _KTEST_EXPECT_IMPL((a) >= (b))
#define ASSERT_GE(a, b) _KTEST_ASSERT_IMPL(EXPECT_GE(a, b))
