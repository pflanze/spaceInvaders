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
example with the `eog` image viewer, or by looking them as text files
(hit return for the next image):

    for f in out/*; do read; cat "$f"; done

### make depend

Running `make depend` requires `makedepend` which is in the
`xutils-dev` package on Debian.
