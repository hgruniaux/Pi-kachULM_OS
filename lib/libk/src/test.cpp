#include <libk/log.hpp>
#include <libk/test.hpp>
#include <libk/utils.hpp>

#ifdef BUILD_TESTS
namespace ktest {
static constexpr size_t _KTEST_MAX_REGISTERED_TESTS = 1024;
static KTest* __ktest_registered_tests[_KTEST_MAX_REGISTERED_TESTS];
static size_t __ktest_registered_test_count = 0;

KTest::KTest(const char* name) {
  __ktest_registered_tests[__ktest_registered_test_count++] = this;
  __ktest_current_name = name;
}

bool KTest::run() {
  do_run();
  if (__ktest_current_ok) {
    libk::print("\x1b[32m[ OK ]\x1b[0m {}", __ktest_current_name);
    return true;
  } else {
    libk::print("\x1b[31m[ KO ]\x1b[0m {} (see above for errors)", __ktest_current_name);
    return false;
  }
}

bool KTest::__ktest_expect(bool cond, std::source_location location) {
  __ktest_current_ok &= cond;

  if (!cond) {
    libk::print("{}:{}: FAILURE", location.file_name(), location.line());
  }

  return !cond;
}

void run_tests() {
  bool is_ok = true;
  for (size_t i = 0; i < __ktest_registered_test_count; ++i) {
    is_ok &= __ktest_registered_tests[i]->run();
  }

  int64_t exit_code = 0;
  if (is_ok) {
    libk::print("All tests passed.");
  } else {
    libk::print("\x1b[31mFAIL.\x1b[0m Some tests failed.");
    exit_code = 1;
  }

  (void)exit_code;
#ifdef TARGET_QEMU
  libk::qemu_exit(exit_code);
#endif  // TARGET_QEMU

  // In case of failure, just halt the system.
  libk::halt();
}
}  // namespace ktest
#endif  // BUILD_TESTS
