#pragma once

#include <atomic>

/**
 * The SpinLock structure is a low-level, mutual-exclusion synchronization primitive that spins while it waits to
 * acquire a lock. On multicore computers, when wait times are expected to be short and when contention is minimal,
 * SpinLock can perform better than other kinds of locks.
 *
 * Moreover, on Kernel space, where mutexes may not be available, SpinLock still can be used.
 *
 * Example:
 * ```c++
 * SpinLock lock;
 * lock.lock(); // Enter critical code
 * // Your code that will be executed only by one thread at any time.
 * lock.unlock(); // Exit critical code
 * ```
 */
class SpinLock {
 public:
  /** @brief Acquires the lock. */
  void lock() {
    // Test and Test-and-Set (TTAS) lock
    // See https://en.wikipedia.org/wiki/Test_and_test-and-set
    // And https://rigtorp.se/spinlock/
    while (m_lock.test_and_set(std::memory_order_acquire))  // acquire lock
    {
      // Since C++20, it is possible to update atomic_flag's
      // value only when there is a chance to acquire the lock.
      // See also: https://stackoverflow.com/questions/62318642
      while (m_lock.test(std::memory_order_relaxed))  // test lock
        spin_wait();                                  // spin
    }
  }

  /** @brief Releases the lock. */
  void unlock() { m_lock.clear(std::memory_order_release); }

 private:
  [[gnu::always_inline]] static void spin_wait() {
    // Inform the CPU that we are doing a spin lock so the kernel could be swapped out to improve overall system
    // performance.
    // See https://www.scs.stanford.edu/~zyedidia/arm64/yield.html And
    // https://cr.openjdk.org/~dchuyko/8186670/yield/spinwait.html
    asm volatile("yield");
  }

  std::atomic_flag m_lock = ATOMIC_FLAG_INIT;
};  // class SpinLock
