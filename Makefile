SRCS=Buttons.c Buttons.h GameEngine.c GameEngine.h Init.c Init.h LED.c LED.h Message.c Message.h Nokia5110.c Nokia5110.h Nokia5110_font.h Sound.c Sound.h SpaceInvaders.c random.h sprite.c sprites.h utils.h

CFLAGS=-O1 -std=c89 -Wall -pedantic 

depend:
	makedepend -- $(CFLAGS) -- $(SRCS)

