# Makefile for the NymphRC test server.
#
# (c) 2017 Nyanko.ws

GCC = g++
MAKEDIR = mkdir -p
RM = rm

ifndef ARCH
ARCH := $(shell g++ -dumpmachine)/
endif

OUTPUT = nymph_server
INCLUDE = -I $(TOP)/src
ifdef NPOCO
LIB := ../../lib/$(ARCH)libnymphrpc.a -lNPocoNet -lNPocoCore
CFLAGS := $(CFLAGS) -DNPOCO
else
LIB := ../../lib/$(ARCH)libnymphrpc.a -lPocoNet -lPocoUtil -lPocoFoundation
endif
CFLAGS := $(INCLUDE) -std=c++11 -DNPOCO=1 -g3
SOURCES := $(wildcard *.cpp)
OBJECTS := $(addprefix obj/,$(notdir) $(SOURCES:.cpp=.o))

# Check for MinGW and patch up POCO
# The OS variable is only set on Windows.
ifdef OS
	CFLAGS := $(CFLAGS) -U__STRICT_ANSI__ -DPOCO_WIN32_UTF8
	#LIB += -lws2_32
else
	LIB += -pthread
endif

all: makedir bin/$(OUTPUT)

makedir:
	$(MAKEDIR) obj
	$(MAKEDIR) bin

obj/%.o: %.cpp
	$(GCC) -c -o $@ $< $(CFLAGS)
	
bin/$(OUTPUT): $(OBJECTS)
	$(GCC) -o $@ $(OBJECTS) $(LIB)

clean:
	$(RM) $(OBJECTS)
	