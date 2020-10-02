# Where to find the files:
VPATH=../src
# From here on do not use the "../src" prefix, relative paths are assumed
# to be found there already.

# ----------------------------------------------------------------------
# Keep these variables updated manually:

SRCS=Buttons.c PRNG.c GameEngine.c Init.c IO.c LED.c Message.c Nokia5110.c Sound.c sounds.c SpaceInvaders.c random.c object.c pp.c stdlib_utils.c test.c
# not including unused modules, for building `testsingle`:
SRCS_TEST=Buttons.c PRNG.c GameEngine.c Message.c Nokia5110.c Sound.c SpaceInvaders.c random.c object.c pp.c stdlib_utils.c test.c

OBJS=Buttons.o PRNG.o GameEngine.o Init.o IO.o Message.o Nokia5110.o Sound.o sounds.o SpaceInvaders.o random.o object.o pp.o stdlib_utils.o test.o

# ----------------------------------------------------------------------
# No need for manual updating here

SAN=`./sanflags`

# To make it possible to work with
#   valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --keep-stacktraces=alloc-and-free ./test
# disable sanitizer via "touch .nosan" or by clearing the SAN variable here: 
#SAN=

CC=`./compiler`
CFLAGS=-fdiagnostics-color=always -std=c99 -Wall -Wextra -Wno-missing-field-initializers -DTEST_WITHOUT_IO -gdwarf-4 -g3 -DEXPORTED= -DDEBUG $(SAN)


all: depend
	make runtest

runtest: test t/out
	./test

t/out:
	mkdir -p out

test: $(OBJS)
	$(CC) $(SAN) -o test $(OBJS)

# depend on OBJS here just to make sure it's rebuilt on any file
# change, since that's the only way dependency info is accessible
# currently:
testsingle.c: $(OBJS)
	./make_testsingle $(SRCS_TEST) > testsingle.c

# (Note: using -I is not optimal, as it makes it look in VPATH for
# *system* header files, too--that's the real meaning for -I, after
# all! Had to rename our assert.h for that reason. Maybe there's
# something more precise?)
testsingle.o: testsingle.c
	$(CC) $(CFLAGS) -UEXPORTED -DEXPORTED=static -I$(VPATH) -c -o testsingle.o testsingle.c

testsingle: testsingle.o
	$(CC) $(SAN) -o testsingle testsingle.o

clean:
	rm -f *.o test testsingle

depend:
	cp Makefile $(VPATH)/
	(cd $(VPATH) && makedepend -- $(CFLAGS) -- $(SRCS)) > .makedepend.log 2>&1
	mv $(VPATH)/Makefile .
	./makedepend_clean Makefile


# DO NOT DELETE

Buttons.o: Buttons.h utils.h
GameEngine.o: GameEngine.h debug.h utils.h object.h sprites.h Nokia5110.h random.h perhaps_assert.h sounds.h Sound.h pp.h
IO.o: IO.h
Init.o: Init.h debug.h
LED.o: LED.h
Message.o: Message.h Nokia5110.h
Nokia5110.o: Nokia5110.h Nokia5110_font.h
PRNG.o: PRNG.h
Sound.o: Sound.h debug.h utils.h IO.h
SpaceInvaders.o: Nokia5110.h Buttons.h utils.h GameEngine.h debug.h object.h random.h Message.h sounds.h Sound.h SpaceInvaders.h pp.h
pp.o: pp.h utils.h object.h
random.o: random.h PRNG.h perhaps_assert.h
sounds.o: debug.h sounds.h Sound.h
stdlib_utils.o: stdlib_utils.h
test.o: Buttons.h utils.h GameEngine.h debug.h object.h Message.h Nokia5110_font.h Nokia5110.h random.h Sound.h SpaceInvaders.h pp.h perhaps_assert.h stdlib_utils.h
