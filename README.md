![alt tag](https://banner2.cleanpng.com/20180622/pc/kisspng-space-invaders-video-game-computer-icons-space-invader-5b2db8bf2434a4.4760147715297230711483.jpg)

José Vidal initially wrote this code as part of the final project
ut610x Embedded Systems Multi-Threaded Interfacing. Christian Jaeger
has written tests, and has reworked it into a roughly object oriented
style, improved the sound subsystem, and has more [plans](TODO.md).

As the project name sugests, the idea was to wite the code for our version of the game (game engine + UI) and the hardware.

The hardware parts were:

* Tiva board
* Nokia screen
* Slider (left right movement)
* 2 buttons
* Speaker for sound: using Binary Weighted Resistors for the DAC.


About the game music:

 *  The software side had 4 groups 8 bit of sound packed into 32 bit
    words, so more sounds could be loaded into the ROM, played back
    using masks.

Demo:

* https://www.youtube.com/watch?v=Jzi1S1Zj7oA

See [HACKING](HACKING.md) for technical details.


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

    for f in out/*; do cat "$f"; read; done

The test also creates *step.dump files which show a dump of the game
state per line, as well as *sound.dump files which additionaly show a
dump of the sound player state for every sound sample. Also, it
creates *sound.raw files which are raw 8 bit unsigned int audio files;
they can be converted to WAV format using:

    sox -r 11010 -c 1 -e unsigned-integer -b 8 1-sound.raw 1-sound.wav

### Dependencies

This needs either gcc or clang, with support for the `-MM`
option. (TCC's `-MD` option might work, too, but will need an adaption
of `bin/cbuild`.)

You need a reasonably recent version of Clang, like `apt install
clang-10`, for the sanitizers to work. (Otherwise disable them by
running `touch .nosan` in `t/`.)

If you get something like:

    /usr/bin/ld: cannot find /usr/lib/llvm-10/lib/clang/10.0.0/lib/linux/libclang_rt.msan-i386.a: No such file or directory

then it's because you're on a 32 bit system--apparently msan only
works on 64 bit systems. The library would be part of the
`libclang-common-9-dev` package for clang-9, for example, which should
be installed already after installing clang-9.

You'll want to install `gdb`.

### make targets

* `runtest`: run the test suite (via `test`); this is the default
    target
* `test`: binary built via separate compilation
* `runtestsingle`: run the test suite via `testsingle`
* `testsingle`: binary built with whole-program analysis

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

