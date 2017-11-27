SRCS=Buttons.c CMWC.c GameEngine.c Init.c LED.c Message.c Nokia5110.c Sound.c SpaceInvaders.c random.c test.c

SAN=`./sanflags`

# To make it possible to work with
#   valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --keep-stacktraces=alloc-and-free ./test
# disable sanitizer here: 
#SAN=
# ^ comment out this line if interested in using the sanitizer

CFLAGS=-O0 -std=c99 -Wall -DTEST_WITHOUT_IO -gdwarf-4 -g3 $(SAN)

OBJS=Buttons.o CMWC.o GameEngine.o Init.o Message.o Nokia5110.o Sound.o SpaceInvaders.o random.o test.o

runtest: test t
	./test

t:
	mkdir t

test: $(OBJS)
	gcc $(SAN) -o test $(OBJS)

clean:
	rm *.o test

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
/usr/include/assert.h:


# DO NOT DELETE

Buttons.o: Buttons.h utils.h
CMWC.o: /usr/include/string.h /usr/include/_ansi.h /usr/include/newlib.h
CMWC.o: /usr/include/_newlib_version.h /usr/include/sys/config.h
CMWC.o: /usr/include/machine/ieeefp.h /usr/include/sys/features.h
CMWC.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
CMWC.o: /usr/include/machine/_types.h /usr/include/machine/_default_types.h
CMWC.o: /usr/include/sys/lock.h /usr/include/sys/cdefs.h
CMWC.o: /usr/include/xlocale.h /usr/include/strings.h
CMWC.o: /usr/include/sys/string.h
GameEngine.o: GameEngine.h sprites.h Nokia5110.h random.h utils.h assert.h
Init.o: Init.h
LED.o: LED.h
Message.o: Message.h Nokia5110.h
Nokia5110.o: Nokia5110.h Nokia5110_font.h
SpaceInvaders.o: Nokia5110.h Init.h Buttons.h utils.h GameEngine.h random.h
SpaceInvaders.o: Message.h
random.o: CMWC.h
test.o: Buttons.h utils.h GameEngine.h Init.h LED.h Message.h
test.o: Nokia5110_font.h Nokia5110.h random.h Sound.h SpaceInvaders.h
test.o: /usr/include/stdio.h /usr/include/_ansi.h /usr/include/newlib.h
test.o: /usr/include/_newlib_version.h /usr/include/sys/config.h
test.o: /usr/include/machine/ieeefp.h /usr/include/sys/features.h
test.o: /usr/include/sys/cdefs.h /usr/include/machine/_default_types.h
test.o: /usr/include/sys/reent.h /usr/include/sys/_types.h
test.o: /usr/include/machine/_types.h /usr/include/sys/lock.h
test.o: /usr/include/sys/types.h /usr/include/sys/_stdint.h
test.o: /usr/include/machine/endian.h /usr/include/machine/_endian.h
test.o: /usr/include/bits/endian.h /usr/include/sys/select.h
test.o: /usr/include/sys/_sigset.h /usr/include/sys/_timeval.h
test.o: /usr/include/sys/timespec.h /usr/include/sys/_timespec.h
test.o: /usr/include/sys/_pthreadtypes.h /usr/include/machine/types.h
test.o: /usr/include/endian.h /usr/include/bits/byteswap.h
test.o: /usr/include/bits/wordsize.h /usr/include/sys/sysmacros.h
test.o: /usr/include/sys/stdio.h /usr/include/string.h /usr/include/xlocale.h
test.o: /usr/include/strings.h /usr/include/sys/string.h /usr/include/errno.h
test.o: /usr/include/sys/errno.h /usr/include/stdlib.h
test.o: /usr/include/machine/stdlib.h /usr/include/alloca.h
