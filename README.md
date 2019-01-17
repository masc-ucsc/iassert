# iassert
A better invariant assertion library with gdb support


This is a C++ library to improve over the default assert in C++. The main advantages:

* Guarded assert support. GI(cond1,cond2) The cond2 triggers a failure only if cond1 is true.

* Release vs Debug spurious messages. The default C assert will result in warnings if a variable is just used in asserts. This source of warnings is removed in iassert.

* GDB support. If you define I_GDB=1 before launch the program, it waits for a GBD connection to remotely debug the program.

* Shorter syntax I(cond) for invariant or always true, GI(cond1,cond2) for guarded asserts

* Allows for messages in asserts, I(cond,"bad bad case") or GI(true,false,"Not again")

* Intercepts segfaults and prints call trace

* Segfault can be intercepted with GDB too.


Example usage:

```cpp
#include <stdio.h>

#include "iassert.hpp"

int main() {
  int i=0;
  int j = 1;

  (void)(i);

  I(false);
  I(true,"this works");
  GI(true,true, "jeje");
  GI(true,true);

  printf("Hello %d\n",j);
}
```

To support segfault call I_setup:

```cpp
#include <stdio.h>

#include "iassert.hpp"

int main() {

  I_setup();

  int bad[10];
  bad[100000] = 0; // Trigger segfault

}
```

To breakpoint with gdb. Write a failing assertion anywhere. Connect with gdb,
set a breakpoint in I_gdb_continuation, now the assertion will continue inside gdb.

```bash
>b I_gdb_continuation
>c
```

