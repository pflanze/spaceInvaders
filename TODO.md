# Todo/aims

(These are Christian's ideas/aims.)

## Primary

### Build for hardware

* Build for the real hardware via gcc or clang instead of Keil

### Sound

* (optionally) actually play sounds via Linux sound system or
  some wrapper

### Testing infrastructure

* add currently playing sounds data (globals) to the game world
  struct, to finish the no-globals work (and so sound can be verified
  via *-step.dump files)
* for working on Sound encoding / infrastructure also write out all
  audio output (as 1 sample per line so diff can pick up time shifts)
  to a separate file

### Browser/WASM

* get it to play in browsers via HTML canvas
    * GUI in HTML/js or React (forget about Nuclear?)

## Secondary

### Sound

* Change sound output to use PWM on a single bit digital output?
  (Could use a simple digital amplifier stage instead of resistor
  network plus analog amplifier. And would probably allow up to 8 bit
  instead of only 4 bit output.)
* Compression (some efficient run-length encoding)
* look at mixer/decoder/streams handling (which is where decompressor
  code will be added)

### Testing infrastructure

* have a GUI that allows to play the game interactively on Linux and
  records all user actions; use the recorded traces as better tests.
* (within that GUI, step back (undo game steps).)

