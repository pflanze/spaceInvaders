# Where to find the files:
VPATH=../src
# From here on do not use the "../src" prefix, relative paths are assumed
# to be found there already.

# ----------------------------------------------------------------------
# Keep these variables updated manually:

SRCS=Buttons.c PRNG.c GameEngine.c Init.c IO.c LED.c Message.c Nokia5110.c Sound.c sounds.c SpaceInvaders.c random.c pp.c stdlib_utils.c stdlib_file.c test.c

OBJS=Buttons.o PRNG.o GameEngine.o Init.o IO.o Message.o Nokia5110.o Sound.o sounds.o SpaceInvaders.o random.o pp.o stdlib_utils.o stdlib_file.o test.o

# ----------------------------------------------------------------------
# No need for manual updating here

SAN=`./sanflags`

# To make it possible to work with
#   valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --keep-stacktraces=alloc-and-free ./test
# disable sanitizer via "touch .nosan" or by clearing the SAN variable here: 
#SAN=

CC=`./compiler`
CFLAGS=-fdiagnostics-color=always -Werror=incompatible-pointer-types -std=c99 -Wall -Wextra -Wno-missing-field-initializers -DTEST_WITHOUT_IO -gdwarf-4 -g3 -DEXPORTED= -DDEBUG $(SAN)


default: runtest

out:
	mkdir -p out

# link
test: $(OBJS)
	$(CC) $(SAN) -o test $(OBJS)

runtest: test out
	./test

# depend on OBJS here just to make sure it's rebuilt on any file
# change, since that's the only way dependency info is accessible
# currently:
testsingle.c: $(OBJS)
	./make_testsingle $(SRCS) > testsingle.c

# (Note: using -I is not optimal, as it makes it look in VPATH for
# *system* header files, too--that's the real meaning for -I, after
# all! Had to rename our assert.h for that reason. Maybe there's
# something more precise?)
testsingle.o: testsingle.c
	$(CC) $(CFLAGS) -UEXPORTED -DEXPORTED=static -I$(VPATH) -c -o testsingle.o testsingle.c

testsingle: testsingle.o
	$(CC) $(SAN) -o testsingle testsingle.o

runtestsingle: testsingle out
	./testsingle


clean:
	rm -f *.o *.d test testsingle


# For normal builds, use autodepend approach from:
# https://scottmcpeak.com/autodepend/autodepend.html

# pull in dependency info for *existing* .o files
-include $(OBJS:.o=.d)

# compile and generate dependency info;
# more complicated dependency computation, so all prereqs listed
# will also become command-less, prereq-less targets
#   sed:    strip the target (everything before colon)
#   sed:    remove any continuation backslashes
#   fmt -1: list words one per line
#   sed:    strip leading spaces
#   sed:    add trailing colons
%.o: %.c
	$(CC) -c $(CFLAGS) $(VPATH)/$*.c -o $*.o
	$(CC) -MM $(CFLAGS) $(VPATH)/$*.c -o $*.d
	@cp -f $*.d $*.d.tmp
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp
