SRCS=Buttons.c GameEngine.c Init.c LED.c Message.c Nokia5110.c Sound.c SpaceInvaders.c sprite.c random.c

CFLAGS=-O1 -std=c99 -Wall -DTEST_WITHOUT_IO

OBJS=Buttons.o GameEngine.o Init.o Message.o Nokia5110.o Sound.o SpaceInvaders.o sprite.o 

test: $(OBJS)
	gcc -o test $(OBJS)

clean:
	rm *.o

depend:
	makedepend -- $(CFLAGS) -- $(SRCS)

# DO NOT DELETE

Buttons.o: Buttons.h utils.h
GameEngine.o: GameEngine.h sprites.h Nokia5110.h random.h
Init.o: Init.h
LED.o: LED.h
Message.o: Message.h Nokia5110.h
Nokia5110.o: Nokia5110.h Nokia5110_font.h
SpaceInvaders.o: Nokia5110.h Init.h Buttons.h utils.h GameEngine.h random.h
SpaceInvaders.o: Message.h
sprite.o: Nokia5110.h
random.o: /usr/include/stdlib.h /usr/include/features.h /usr/include/alloca.h
