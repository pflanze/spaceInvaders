# Where to find the files:
VPATH=../src
# From here on do not use the "../src" prefix, relative paths are assumed
# to be found there already.

SAN:=`./sanflags`

# To make it possible to work with
#   valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --keep-stacktraces=alloc-and-free ./test
# disable sanitizer via "touch .nosan" or by clearing the SAN variable here: 
#SAN:=

CC:=`./compiler`
CFLAGS=-fdiagnostics-color=always -Werror=incompatible-pointer-types -std=c99 -Wall -Wextra -Wno-missing-field-initializers -DTEST_WITHOUT_IO -gdwarf-4 -g3 -DEXPORTED= -DDEBUG $(SAN)


default: runtest

.PHONY: test
test:
	CFLAGS="$(CFLAGS)" CC="$(CC)" VPATH="$(VPATH)" ../bin/cbuild test

.PHONY: runtest
runtest: test
	./test


ALLSRCS:=$(patsubst $(VPATH)/%,%,$(wildcard $(VPATH)/*.c $(VPATH)/*.h))
SRCS:=$(patsubst $(VPATH)/%,%,$(wildcard $(VPATH)/*.c))

testsingle.c: $(ALLSRCS)
	./make_testsingle $(SRCS) > testsingle.c

# (Note: using -I is not optimal, as it makes it look in VPATH for
# *system* header files, too--that's the real meaning for -I, after
# all! Had to rename our assert.h for that reason. Maybe there's
# something more precise?)
testsingle.o: testsingle.c
	$(CC) $(CFLAGS) -UEXPORTED -DEXPORTED=static -I$(VPATH) -c -o testsingle.o testsingle.c

testsingle: testsingle.o
	$(CC) $(SAN) -o testsingle testsingle.o

.PHONY: runtestsingle
runtestsingle: testsingle
	./testsingle


.PHONY: clean
clean:
	rm -f *.o *.d test testsingle testsingle.c

