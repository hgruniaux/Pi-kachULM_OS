# How to debug the kernel?

## The debug library

A small library is provided in the kernel source tree to simplify debugging. Use and overuse it.

**TL;DR.** This library is located at `debug.hxx` and contains two main facilities:
- assertions with the macro `KASSERT(cond)`
- logging with the macros `LOG_INFO`, `LOG_ERROR`, etc.

### Assertions

The header file `debug.hxx` provides a `KASSERT` macro that acts just like the C macro `assert()` from the standard library. Use it everywhere. It is better to assert something that you expected than having silent bugs hard to debug.

Example:
```c++
void my_func(int x) {
    KASSERT(x > 42);
    ...
}
```

### Logging

#### How to use it.

The debug library allows you to log information during the execution of the kernel. There are different levels of severities available:
- `debug::Severity::Trace`: To tell what is currently executed or at what point the kernel is.
- `debug::Severity::Debug`: Information only useful for debugging some specific part.
- `debug::Severity::Info`: Some useful information.
- `debug::Severity::Warning`: A warning.
- `debug::Severity::Error`: An error in the code. Something went wrong that was unexpected.
  The kernel should be able to recover from this.
- `debug::Severity::Critical`: Highest level. An error that cannot be recovered from.

You can either use the C++ functions:
- `debug::trace()`, `debug::info()`, ..., `debug::critical()`
  Or the C++ macros:
- `LOG_TRACE()`, ..., `LOG_CRITICAL()`.

Example:
```c++
void my_func() {
    LOG_TRACE("my_category", "entering the function my_func()");

    if (oops) {
        LOG_WARNING("my_category", "oops should be false for better performance!");
    }
}
```

`my_category` can be anything. It should describe a part of the kernel from which the log comes from.
For example, the driver or the kernel module (`usb`, `memory`, `uart`, etc.). You can also set it to `nullptr` to not
print the category.

#### How to control the log level.

To control what log message is emitted, there are two ways:
- define the `LOG_MIN_LEVEL` to one of `LOG_*_LEVEL` where `*` is one of `TRACE`, `DEBUG`, ..., `CRITICAL`. This changes the definition of the `LOG_*` macros but do not filter the logs directly emitted using the `debug::*()` functions. The filtering is done at compile time.
- call `debug::set_level()` with a `debug::Severity`. This also works for logs emitted directly using the `debug::*()` functions. The filtering is done at run time.
Example:
```c++
#define LOG_MIN_LEVEL LOG_ERROR_LEVEL
#include "debug.hxx"

// The following will be expanded to nothing.
LOG_WARNING("usb", "unknown device");
// The following will correctly expanded to debug::error(...)
LOG_ERROR("video", "framebuffer too big");

// Or at run time:
debug::set_level(debug::Severity::Error);
// Then all logs with a severity level less than Error will not be emitted.
```

#### How to access it.

The logs are written to UART0 by the debug library. Therefore, you can read it
in the serial output of QEMU (if you passed `-serial stdio` to QEMU).

**Warning**: If you use CLion and start QEMU from a run configuration, do not forget to tick the `Emulate terminale` checkbox
to correctly the output.
