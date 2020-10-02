# Todo

(Corresponding to "Aims" below.)

1.  Get to work on real hardware via gcc/clang.
1.  Verify sound is still working (how, without DAC?).
1.  Rework sound output to PWM
1.  Can we get rid of `jk`?
1.  Get rid of `origConsts` if possible (play sound immediately, so it
    won't need the change; perhaps move to functional approach (next
    vs. old state during a game cycle)? But requires double the
    RAM.)
1.  Move from initialization and separate `alive` flag to just
    _construct and _destruct (assumes actor object life times can be
    equal to actor life time). If `vtable` is used, don't need an
    alive flag anymore, otherwise we will for safety (but then that's
    a debug feature, so, only rely on `vtable`, OK?).

(Some work for audio compression is ongoing outside this repo and
might be included after the above.)

# Aims

(These are Christian's ideas/aims.)

## Primary

### Build for hardware

* Build for the real hardware via gcc or clang instead of Keil

### Browser/WASM

* get it to play in browsers via HTML canvas
    * GUI in HTML/js or React (forget about Nuclear?)

## Secondary

### Sound

* play sounds via Linux sound system or some wrapper
* Change sound output to use PWM on a single bit digital output?
  (Could use a simple digital amplifier stage instead of resistor
  network plus analog amplifier. And would probably allow up to 8 bit
  instead of only 4 bit output.)
* Instead of taking the 4 upper bits from the original 8-bit sound
  samples, use a u-law style encoding to 4 (or more?) bits.
* Compression (some efficient run-length encoding)
* look at mixer/decoder/streams handling (which is where decompressor
  code will be added)

### Testing infrastructure

* have a GUI that allows to play the game interactively on Linux and
  records all user actions; use the recorded traces as better tests.
* (within that GUI, step back (undo game steps).)


# Various

