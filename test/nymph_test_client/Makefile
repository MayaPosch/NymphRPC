# Makefile for the NymphRC test client.
#
# (c) 2017 Nyanko.ws

GCC = g++
MAKEDIR = mkdir -p
RM = rm

OUTPUT = nymph_client
INCLUDE = -I $(TOP)/src
LIB := -L ../../lib -lnymphrpc -lPocoNet -lPocoUtil -lPocoFoundation -lPocoJSON
CFLAGS := $(INCLUDE) -std=c++11 -g3

# Check for MinGW and patch up POCO
# The OS variable is only set on Windows.
ifdef OS
	CFLAGS := $(CFLAGS) -U__STRICT_ANSI__
	#LIB += -lws2_32
else
	LIB += -pthread
endif

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
	