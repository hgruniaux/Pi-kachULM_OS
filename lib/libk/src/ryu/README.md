# Implementation of the Ryu algorithm

The Ryu algorithm is used to convert `float`/`double` to string.

The implementation provided in this folder comes from the GitHub repository:
https://github.com/ulfjack/ryu/tree/master.

It is licensed under the terms of the Apache-2.0 license or the BSL-1.0 license.

The code original implementation is patched to be supported inside the kernel. Some functions were removed (the ones
using `malloc`).
Moreover, all standard library calls were replaced by calls to the libk.
