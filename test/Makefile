#
# Makefile for benchmark
#
#

GCC     = g++
MAKEDIR = mkdir -p

INCLUDE      = -I ../src
LIBS        := -lPocoNet -lPocoUtil -lPocoFoundation -lPocoJSON 
CFLAGS      := $(INCLUDE) -std=c++11 -O2

ifndef OS
	LIBS += -pthread
endif

LIB_SOURCES := $(wildcard ../src/*.cpp)
LIB_OBJECTS := $(subst    ../src/, obj/, $(LIB_SOURCES:.cpp=.o))
LIB_OUTPUT   = libnymphrpc

BMK_SOURCES := $(wildcard  *.cpp)
BMK_OUTPUT  := $(BMK_SOURCES:.cpp=)
BMK_LIBS    := lib/$(LIB_OUTPUT).a ${LIBS}

#
# Targets:
#

all: lib benchmark run

lib: makedir-lib lib/$(LIB_OUTPUT).a

benchmark: makedir-bmk bin/${BMK_OUTPUT}

run:
	bin/${BMK_OUTPUT} --benchmark-no-analysis --benchmark-samples 20

# lib targets:

obj/%.o: ../src/%.cpp
	$(GCC) -c -o $@ $< $(CFLAGS)

lib/$(LIB_OUTPUT).a: $(LIB_OBJECTS)
	-rm -f $@
	$(AR) rcs $@ $^

makedir-lib:
	$(MAKEDIR) lib/
	$(MAKEDIR) obj/

# benchmark targets:

bin/${BMK_OUTPUT}: ${BMK_SOURCES} lib/$(LIB_OUTPUT).a
	$(GCC) -o $@ $< $(CFLAGS) ${BMK_LIBS}

makedir-bmk:
	$(MAKEDIR) bin/

# clean targets:

clean: clean-lib clean-bmk

dist-clean: clean clean-lib-lib clean-bmk-bin

real-clean: dist-clean

clean-lib:
	$(RM) $(LIB_OBJECTS)

clean-bmk:
	$(RM) $(BMK_OBJECTS)

clean-lib-lib:
	$(RM) lib/$(LIB_OUTPUT).a

clean-bmk-bin:
	$(RM) bin/${BMK_OUTPUT}

# end of file
