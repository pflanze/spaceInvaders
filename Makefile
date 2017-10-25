SRCS=Buttons.c CMWC.c GameEngine.c Init.c LED.c Message.c Nokia5110.c Sound.c SpaceInvaders.c random.c test.c

SAN=`./sanflags`

CFLAGS=-O1 -std=c99 -Wall -DTEST_WITHOUT_IO -gdwarf-4 -g3 $(SAN)

OBJS=Buttons.o CMWC.o GameEngine.o Init.o Message.o Nokia5110.o Sound.o SpaceInvaders.o random.o test.o

runtest: test t
	./test

t:
	mkdir t

test: $(OBJS)
	gcc $(SAN) -o test $(OBJS)

clean:
	rm *.o

depend:
	makedepend -- $(CFLAGS) -- $(SRCS)

# hack to ignore those parts from the auto-generated directions below:
/usr/include/stdlib.h:
/usr/include/features.h:
/usr/include/alloca.h:
/usr/include/signal.h:
/usr/include/time.h:
/usr/include/stdio.h:
/usr/include/libio.h:
/usr/include/_G_config.h:
/usr/include/wchar.h:
/usr/include/string.h:
/usr/include/xlocale.h:
/usr/include/errno.h:
/usr/include/stdlib.h:
/usr/include/alloca.h:

# DO NOT DELETE

Buttons.o: Buttons.h utils.h
CMWC.o: /usr/include/string.h /usr/include/features.h /usr/include/xlocale.h
GameEngine.o: GameEngine.h sprites.h Nokia5110.h random.h utils.h
Init.o: Init.h
LED.o: LED.h
Message.o: Message.h Nokia5110.h
Nokia5110.o: Nokia5110.h Nokia5110_font.h
SpaceInvaders.o: Nokia5110.h Init.h Buttons.h utils.h GameEngine.h random.h
SpaceInvaders.o: Message.h
random.o: CMWC.h
test.o: trap.h /usr/include/signal.h /usr/include/features.h
test.o: /usr/include/time.h Buttons.h utils.h GameEngine.h Init.h LED.h
test.o: Message.h Nokia5110_font.h Nokia5110.h random.h Sound.h
test.o: SpaceInvaders.h /usr/include/stdio.h /usr/include/libio.h
test.o: /usr/include/_G_config.h /usr/include/wchar.h /usr/include/string.h
test.o: /usr/include/xlocale.h /usr/include/errno.h /usr/include/stdlib.h
test.o: /usr/include/alloca.h
