![alt tag](https://banner2.cleanpng.com/20180622/pc/kisspng-space-invaders-video-game-computer-icons-space-invader-5b2db8bf2434a4.4760147715297230711483.jpg)

I wrote this code as part of the final project ut610x Embedded Systems Multi-Threaded Interfacing

As the project name sugests, the idea was to wite the code for our version of the game (game engine + UI) and the hardware.
The harware parts were:
* Tiva board
* Nokia screen
* Slider (left right movement)
* 2 buttons
* Speaker for sound: using Binary Weighted Resistors for the DAC.


About the game music:
* The software side had 4 groups 8 bit of sound packed into 32 bit words, so I could load more sounds into ROM.... and play it back using masks.

Demo:
* https://www.youtube.com/watch?v=Jzi1S1Zj7oA

## Automatic tests

The code can be built on Linux and perhaps Cygwin/MSYS to run
integration tests.

    cd t
    make runtest

This compiles and runs with (some of) gcc's sanitizers if
available; this can be turned off via:

    cd t
    touch .nosan

The test creates image files in `t/out/` which can be viewed for
example with the `eog` image viewer, or by looking at them as text
files (hit return for the next image):

    for f in out/*; do read; cat "$f"; done

### Dependencies

You need a reasonably recent version of Clang, like `apt install
clang-10`, for the sanitizers to work. (Otherwise disable them by
running `touch .nosan` in `t/`.)

Running `make depend` requires `makedepend` which is in the
`xutils-dev` package on Debian.

If you get something like:

    /usr/bin/ld: cannot find /usr/lib/llvm-10/lib/clang/10.0.0/lib/linux/libclang_rt.msan-i386.a: No such file or directory

then it's because you're on a 32 bit system--apparently msan only
works on 64 bit systems. The library would be part of the
`libclang-common-9-dev` package for clang-9, for example, which should
be installed already after installing clang-9.

You'll want to install `gdb`.

### make targets

* `depend`: update the Makefile for changes in cross file dependencies
    (header file includes)
* `runtest`: run the test suite (via `test`)
* `test`: binary built via separate compilation
* `testsingle`: binary built with whole-program analysis
* default: runs `make depend` then `make runtest`

### Compiler configuration

By default, it tries to find clang, and will compile with -O0 and with
sanitizers enabled. This can be changed via (run from within `t`
directory):

* use gcc instead of clang: `touch .use-gcc`
* use different optimization level: `echo -Os > .opt`

Run `./compiler` to see compiler and optimization option choice (this
does not reflect sanitizer options though--TODO simplify).

* disable sanitizer: `touch .nosan`

In all cases, after changing options, `make clean` has to be run
before compiling again or it will reuse existing object files.

