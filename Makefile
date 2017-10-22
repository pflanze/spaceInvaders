SRCS=Buttons.c Buttons.h GameEngine.c GameEngine.h Init.c Init.h LED.c LED.h Message.c Message.h Nokia5110.c Nokia5110.h Nokia5110_font.h Sound.c Sound.h SpaceInvaders.c random.h sprite.c sprites.h utils.h

CFLAGS=-O1 -std=c89 -Wall -pedantic 

depend:
	makedepend -- $(CFLAGS) -- $(SRCS)

# DO NOT DELETE

Buttons.o: Buttons.h utils.h
Buttons.o: utils.h
GameEngine.o: GameEngine.h sprites.h Nokia5110.h
Init.o: Init.h
Message.o: Message.h Nokia5110.h
Nokia5110.o: Nokia5110.h Nokia5110_font.h
Sound.o: Sound.h
SpaceInvaders.o: Nokia5110.h Init.h Buttons.h utils.h GameEngine.h random.h
SpaceInvaders.o: Message.h
sprite.o: Nokia5110.h
