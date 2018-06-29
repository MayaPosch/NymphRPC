# Makefile for the NymphRC benchmark.
#
# (c) 2017 Nyanko.ws

GCC = g++
MAKEDIR = mkdir -p
RM = rm

OUTPUT = nymphbench
INCLUDE = -I $(TOP)/src
LIB := -L ../../lib -lnymphrpc -lPocoNet -lPocoUtil -lPocoFoundation -lPocoJSON
#-DPOCO_WIN32_UTF8
CFLAGS := $(INCLUDE) -std=c++11 -U__STRICT_ANSI__ -g3 
#SOURCES := $(wildcard *.cpp)
SOURCES := nymphbench_with_fixture.cpp nymphclientclass.cpp
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
	$(RM) $(OBJECTS) bin/$(OUTPUT)
