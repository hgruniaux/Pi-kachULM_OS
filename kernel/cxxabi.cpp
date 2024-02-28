/*
 * cxxabi.cxx
 *
 * Implements part of the Itanium C++ ABI to be used in the kernel.
 */

#include <cstddef>
#include <cstdint>

// See https://itanium-cxx-abi.github.io/cxx-abi/abi.html.

/// A standard entry point that a compiler may reference in virtual tables
/// to indicate a pure virtual function.
extern "C" [[noreturn]] void
__cxa_pure_virtual()
{
  // From Itanium C++ ABI:
  //  > This routine will only be called if the user calls a non-overridden pure
  //  > virtual function, which has undefined behavior according to the C++
  //  > Standard. Therefore, this ABI does not specify its behavior, but it is
  //  > expected that it will terminate the program, possibly with an error
  //  > message.

  // TODO: replace this by a call to kernel panic (debug::panic).
  while (true)
    ;
}

/// A standard entry point that a compiler will reference in virtual tables to
/// indicate a deleted virtual function.
extern "C" [[noreturn]] void
__cxa_deleted_virtual()
{
  // From Itanium C++ ABI:
  //  > This routine shall not return and while this ABI does not otherwise
  //  > specify its behavior, it is expected that it will terminate the program,
  //  > possibly with an error message.

  // TODO: replace this by a call to kernel panic (debug::panic).
  while (true)
    ;
}

// FIXME: The following __cxa_guard_*() functions should be thread safe. We
//        can simply use atomics (the <atomic.h> or <atomic> headers are
//        available in freestanding env).

extern "C" int
__cxa_guard_acquire(int64_t* guard_object)
{
  // From Itanium C++ ABI:
  //  > Returns 1 if the initialization is not yet complete; 0 otherwise. This
  //  > function is called before initialization takes place. If this function
  //  > returns 1, either __cxa_guard_release or __cxa_guard_abort must be
  //  > called with the same argument. The first byte of the guard_object is not
  //  > modified by this function.

  return !*guard_object;
}

extern "C" void
__cxa_guard_release(int64_t* guard_object)
{
  // From Itanium C++ ABI:
  //  > Sets the first byte of the guard object to a non-zero value. This
  //  > function is called after initialization is complete.

  *guard_object = 1;
}

extern "C" void
__cxa_guard_abort(int64_t*)
{
  // From Itanium C++ ABI:
  //  > This function is called if the initialization terminates by throwing an
  //  > exception.
}

struct AtExitEntry
{
  void (*destructor)(void*);
  void* object;
}; // struct AtExitEntry

/// The maximum count of functions that can be registered at the same
/// time with __cxa_atexit().
static constexpr size_t kMaxAtExitFunctions = 128;
static AtExitEntry atexit_entries[kMaxAtExitFunctions];
static size_t atexit_entries_count = 0;

extern "C" int
__cxa_atexit(void (*f)(void*), void* p, void*)
{
  // See https://itanium-cxx-abi.github.io/cxx-abi/abi.html#dso-dtor-runtime-api

  if (atexit_entries_count >= kMaxAtExitFunctions)
    return -1;

  atexit_entries[atexit_entries_count].destructor = f;
  atexit_entries[atexit_entries_count].object = p;
  // We ignore the last argument (the DSO handle) as it is unused.
  atexit_entries_count++;
  return 0;
}

extern "C" void
__cxa_finalize(void* f)
{
  // See https://itanium-cxx-abi.github.io/cxx-abi/abi.html#dso-dtor-runtime-api

  size_t i = atexit_entries_count;

  if (f == nullptr) {
    // According to the Itanium C++ ABI, if f is null then we must call
    // all destructors (noy yet called).
    while (i--) {
      if (atexit_entries[i].destructor != nullptr) {
        atexit_entries[i].destructor(atexit_entries[i].object);
      }
    }

    atexit_entries_count = 0;
    return;
  }

  while (i--) {
    if (atexit_entries[i].destructor == f) {
      atexit_entries[i].destructor(atexit_entries[i].object);
      atexit_entries[i].destructor = nullptr;
    }
  }

  // Cleaning up the entries array. We remove all entries with a null
  // destructor and update atexit_entries_count accordingly. We also try
  // to avoid "holes" in the table.
  const size_t old_atexit_entries_count = atexit_entries_count;
  for (size_t i = 0; i < old_atexit_entries_count; ++i) {
    if (atexit_entries[i].destructor != nullptr)
      continue;

    for (size_t j = i + 1; j < atexit_entries_count; ++j) {
      atexit_entries[j - 1] = atexit_entries[j];
    }

    atexit_entries[atexit_entries_count - 1].destructor = nullptr;
    --atexit_entries_count;
  }
}
