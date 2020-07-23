I wrote this code as part of the final project ut610x Embedded Systems Multi-Threaded Interfacing

As the project name sugests, the idea was to wite the code for our version of the game (game engine + UI) and the hardware.
The harware parts were:
	-Tiva board
	-Nokia screen
	-Slider (left right movement)
	-2 buttons
	-Speaker for sound: using Binary Weighted Resistors for the DAC.


About the game music:
The software side had 4 groups 8 bit of sound packed into 32 bit words, so I could load more sounds into ROM.... and play it back using masks.

Demo:
https://www.youtube.com/watch?v=Jzi1S1Zj7oA