# Where to find the files:
VPATH=..
# From here on do not use the "../" prefix, relative paths are assumed
# to be found in ".." already.

# ----------------------------------------------------------------------
# Keep these two variables updated manually:

SRCS=Buttons.c CMWC.c GameEngine.c Init.c LED.c Message.c Nokia5110.c Sound.c SpaceInvaders.c random.c test.c

OBJS=Buttons.o CMWC.o GameEngine.o Init.o Message.o Nokia5110.o Sound.o SpaceInvaders.o random.o test.o

# ----------------------------------------------------------------------
# No need for manual updating here

SAN=`./sanflags`

# To make it possible to work with
#   valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --keep-stacktraces=alloc-and-free ./test
# disable sanitizer via "touch .nosan" or by clearing the SAN variable here: 
#SAN=

CC=gcc
CFLAGS=-O0 -std=c99 -Wall -DTEST_WITHOUT_IO -gdwarf-4 -g3 $(SAN)


all: depend
	make runtest

runtest: test t
	./test

t:
	mkdir t

test: $(OBJS)
	$(CC) $(SAN) -o test $(OBJS)

clean:
	rm *.o test

depend:
	cp Makefile $(VPATH)/
	(cd $(VPATH) && makedepend -- $(CFLAGS) -- $(SRCS)) > .makedepend.log 2>&1
	mv $(VPATH)/Makefile .
	./makedepend_clean Makefile


# DO NOT DELETE

Buttons.o: Buttons.h utils.h
GameEngine.o: GameEngine.h sprites.h Nokia5110.h random.h utils.h assert.h
Init.o: Init.h
LED.o: LED.h
Message.o: Message.h Nokia5110.h
Nokia5110.o: Nokia5110.h Nokia5110_font.h
SpaceInvaders.o: SpaceInvaders.h Nokia5110.h Init.h Buttons.h utils.h
SpaceInvaders.o: GameEngine.h random.h Message.h
random.o: random.h CMWC.h
test.o: Buttons.h utils.h GameEngine.h Init.h LED.h Message.h
test.o: Nokia5110_font.h Nokia5110.h random.h Sound.h SpaceInvaders.h
