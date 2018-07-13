# Makefile for the NymphRC benchmark.
#
# (c) 2017 Nyanko.ws

GCC = g++
MAKEDIR = mkdir -p
RM = rm

OUTPUT = rpcbench
INCLUDE = -I ./
LIB := -L ./ -lrpc -lpthread
CFLAGS := $(INCLUDE) -std=c++11 -O1 -g3
#SOURCES := $(wildcard *.cpp)
SOURCES := rpcbench_fixture.cpp
OBJECTS := $(addprefix obj/,$(notdir) $(SOURCES:.cpp=.o))

all: makedir $(OBJECTS) bin/$(OUTPUT)

makedir:
	$(MAKEDIR) obj
	$(MAKEDIR) bin

obj/%.o: %.cpp
	$(GCC) -c -o $@ $< $(CFLAGS)
	
bin/$(OUTPUT):
	$(GCC) -o $@ $(OBJECTS) $(LIB)

clean:
	$(RM) $(OBJECTS)
	
