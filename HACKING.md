# Hints for Hacking

## Various

  * All non-static procedure definitions are prefixed with the
    `EXPORTED` macro. This macro expands to nothing when doing
    separate compilation (`make test`), and to `static` when compiling
    all files in a single unit (`make testsingle`). It is needed for
    this hacky way to achieve whole system optimization (so that the
    compiler can potentially do better optimizations than LTO allows,
    and so it doesn't need to provide functions as external symbols to
    potentially allow it to generate smaller code). (Please tell us if
    there's a better way to achieve the same.)
    
## Memory management

All memory for the game is (currently) statically allocated for the
release build for the actual hardware. The tests, and possible future
ports to WASM supporting multiple instances of the game playing at
once, allocate things on the stack or heap.

## Object orientation

Struct types use names with upper case initial. Variable and procedure
names use lower case initials.

### Static dispatch

Procedures that act on a particular struct type, i.e. methods, are
named like `TypeName_procedureName` or `TypeName_procedure_name`
(TODO: use one of those two consistently?). The first argument to a
method is of type `struct TypeName *` and named `this`.

Object initializers are such methods and called `TypeName_init`
(better suggestion?). Those which benefit from something akin to named
arguments take a struct (use a compound literal!) of the same type as
an argument and are declared `static inline` so that the compiler can
optimize the construction of the compound literal. Note: you can leave
away mentions of fields in the compound literal and they will be
initialized to zero, apparently guaranteed by the C standard (TODO:
double check?); `-Wno-missing-field-initializers` is passed to the
compilers to avoid seeing warnings about such missing fields.

### Dynamic dispatch (virtual methods)

This isn't really needed for Space Invaders (in its current state),
but it's neat for pretty printing ([`pp.h`](src/pp.h)), especially
since `PP` can used in GDB on any struct that implements the
"ObjectInterface" (which is defined in [`object.h`](src/object.h)),
also it's necessary in [`stdlib_file.c`](src/stdlib_file.c) as part of
the testing infrastructure, to implement `OutFile_xclose_and_free` (to
avoid code duplication via a virtual method call).

A virtual call is carried out via the `VCALL` macro. `PP`, `PP_TO` and
`PP_TO_` are convenience macros that make it even simpler.

Dynamic dispatch is implemented via a field `vtable` in the struct,
which points to a struct with the method implementations for instances
of this struct.

To avoid compiler warnings without needing an explicit (dangerous)
cast, wrapper procedures (e.g. `_Actor_pp`) are used between a method
implementation (e.g. `Actor_pp`) and their use in the method table
definition (e.g. `Actor_ObjectInterface`).

There's still a cast needed for upcasting (implementing a
procedure/method that works for multiple subclasses).

`LET_NEW` from [`object.h`](src/object.h) can be used to
heap-allocate a struct type that has a vtable field and whose vtable
exists in the variable named by the struct type name with `_VTable`
appended. I.e. 

    LET_NEW(this, Foo);

will [`xmalloc`](src/stdlib_utils.h) a `struct Foo`, assign it to the
new `struct Foo *this` variable, and set its `vtable` field to
`&Foo_VTable`.


## Pretty printing

Already mentioned under "Dynamic dispatch". See `pp.h`. It can be used
for debugging in GDB, but more so to print internal state to files
during test runs. Not actually really pretty in the usual sense since
it prints to a single line only. Doing this is better for testing, as
diffs across one line per item are easier to follow for changes during
the execution flow of a program--see e.g. `gitk t/1-step.dump`, and
switch from "line diff" to "color words".

Since pretty printing isn't needed for a production build (on the
actual hardware), to save space it is being excluded if the `DEBUG`
switch is off. (XX might be needed as replacement if the `alive` flag
is removed.)

## Debugging

  * As mentioned in the README, by default the test build uses
    sanitizers to detect initialization and (maybe) more issues.
  * There's a `WARN` macro in `debug.h` that is conditional on the
    `DEBUG` switch and includes file/line location info.
  * The aforementioned dump files like `t/1-step.dump` should help pin
    down issues.
  * Gcc does retain macros as part of the debugging symbols, meaning
    that with that compiler you can use macros from GDB with the given
    Makefile.
  * (Note: `gdbrun` from [chj-bin](https://github.com/pflanze/chj-bin)
    might be handy.)

