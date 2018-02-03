# Makefile for the RPCLib test server.
#
# (c) 2017 Nyanko.ws

GCC = g++
MAKEDIR = mkdir -p
RM = rm

OUTPUT = rpc_server
INCLUDE = -I /usr/local/include
LIB := -L /usr/local/lib -lrpc -lws2
CFLAGS := $(INCLUDE) -std=c++11 -O1
SOURCES := $(wildcard *.cpp)
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
	
