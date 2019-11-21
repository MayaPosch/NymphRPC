# Makefile for the NymphRC library.
#
# Allows one to build the library as a .a file, or
# build the test server and client.
#
# (c) 2017 Nyanko.ws

export TOP := $(CURDIR)

ifdef ANDROID
TOOLCHAIN_PREFIX := arm-linux-androideabi-
ifdef OS
TOOLCHAIN_POSTFIX := .cmd
endif
GCC = $(TOOLCHAIN_PREFIX)g++$(TOOLCHAIN_POSTFIX)
MAKEDIR = mkdir -p
RM = rm
AR = $(TOOLCHAIN_PREFIX)ar
else
GCC = g++
MAKEDIR = mkdir -p
RM = rm
AR = ar
endif

OUTPUT = libnymphrpc.a
INCLUDE = -I src
#-DPOCO_WIN32_UTF8
CFLAGS := $(INCLUDE) -g3 -std=c++11 -O0

ifdef ANDROID
CFLAGS += -fPIC
endif

# Check for MinGW and patch up POCO
# The OS variable is only set on Windows.
ifdef OS
	CFLAGS := $(CFLAGS) -U__STRICT_ANSI__
	LIBS += -lws2_32
else
	LIBS += -pthread
endif

SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(addprefix obj/,$(notdir) $(SOURCES:.cpp=.o))

all: lib test

lib: makedir $(OBJECTS) lib/$(OUTPUT)
	
obj/%.o: %.cpp
	$(GCC) -c -o $@ $< $(CFLAGS)
	
lib/$(OUTPUT): $(OBJECTS)
	-rm -f $@
	$(AR) rcs $@ $^
	
makedir:
	$(MAKEDIR) lib
	$(MAKEDIR) obj
	$(MAKEDIR) obj/src
	
test: test-client test-server
	
test-client:
	$(MAKE) -C ./test/nymph_test_client
	
test-server:
	$(MAKE) -C ./test/nymph_test_server

clean: clean-lib clean-test

clean-test: clean-test-client clean-test-server

clean-lib:
	$(RM) $(OBJECTS)
	
clean-test-client:
	$(MAKE) -C ./test/nymph_test_client clean
	
clean-test-server:
	$(MAKE) -C ./test/nymph_test_server clean
	