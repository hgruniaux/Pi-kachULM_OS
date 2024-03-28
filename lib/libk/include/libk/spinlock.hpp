#pragma once

#include <atomic>

#include "utils.hpp"

namespace libk {
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
        spin_wait();
    }
  }

  /** @brief Releases the lock. */
  void unlock() { m_lock.clear(std::memory_order_release); }

 private:
  [[gnu::always_inline]] static inline void spin_wait() {
    // Inform the CPU that we are doing a spin lock so the kernel could be swapped out to improve overall system
    // performance.
    // See https://www.scs.stanford.edu/~zyedidia/arm64/yield.html And
    // https://cr.openjdk.org/~dchuyko/8186670/yield/spinwait.html
    yield();
  }

  std::atomic_flag m_lock = ATOMIC_FLAG_INIT;
};  // class SpinLock

/**
 * @brief RAII wrapper around SpinLock.
 *
 * This class should work like std::lock_guard but for SpinLock.
 *
 * Example:
 * ```c++
 * {
 *     SpinLockGuard guard(my_spinlock); // the spin lock is acquired here
 *     // code...
 * } // the spin lock is released here
 * ```
 */
class SpinLockGuard {
 public:
  SpinLockGuard() = default;

  explicit SpinLockGuard(SpinLock& spinlock) {
    m_spinlock = &spinlock;
    m_spinlock->lock();
  }

  ~SpinLockGuard() {
    if (m_spinlock == nullptr)
      return;

    m_spinlock->unlock();
  }

  // Move semantic
  SpinLockGuard(SpinLockGuard&& guard) : m_spinlock(guard.m_spinlock) { guard.m_spinlock = nullptr; }
  SpinLockGuard& operator=(SpinLockGuard&& guard) {
    release();
    m_spinlock = guard.m_spinlock;
    guard.m_spinlock = nullptr;
    return *this;
  }

  // Disable copy
  SpinLockGuard(const SpinLockGuard&) = delete;
  SpinLockGuard& operator=(const SpinLockGuard&) = delete;

  /** @brief Gets the underlying spinlock. */
  [[nodiscard]] SpinLock* get_spinlock() { return m_spinlock; }
  [[nodiscard]] const SpinLock* get_spinlock() const { return m_spinlock; }

  /** @brief Unlocks and releases the underlying spinlock (if any). */
  void release() {
    if (m_spinlock == nullptr)
      return;

    m_spinlock->unlock();
    m_spinlock = nullptr;
  }

 private:
  SpinLock* m_spinlock = nullptr;
};  // class ScopedSpinLock
}  // namespace libk
