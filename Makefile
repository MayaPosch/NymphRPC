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

OUTPUT = libnymphrpc
VERSION = 0.1
INCLUDE = -I src
LIBS := -lPocoNet -lPocoUtil -lPocoFoundation -lPocoJSON 
CFLAGS := $(INCLUDE) -g3 -std=c++11 -O0
SHARED_FLAGS := -fPIC -shared -Wl,-soname,$(OUTPUT).so.$(VERSION)

ifdef ANDROID
CFLAGS += -fPIC
endif

# Check for MinGW and patch up POCO
# The OS variable is only set on Windows.
ifdef OS
	CFLAGS := $(CFLAGS) -U__STRICT_ANSI__ -DPOCO_WIN32_UTF8
	LIBS += -lws2_32
else
	LIBS += -pthread
endif

SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(addprefix obj/static/,$(notdir) $(SOURCES:.cpp=.o))
SHARED_OBJECTS := $(addprefix obj/shared/,$(notdir) $(SOURCES:.cpp=.o))

all: lib test

lib: makedir lib/$(OUTPUT).a lib/$(OUTPUT).so.$(VERSION)
	
obj/static/%.o: %.cpp
	$(GCC) -c -o $@ $< $(CFLAGS)
	
obj/shared/%.o: %.cpp
	$(GCC) -c -o $@ $< $(SHARED_FLAGS) $(CFLAGS) $(LIBS)
	
lib/$(OUTPUT).a: $(OBJECTS)
	-rm -f $@
	$(AR) rcs $@ $^
	
lib/$(OUTPUT).so.$(VERSION): $(SHARED_OBJECTS)
	$(GCC) -o $@ $(CFLAGS) $(SHARED_FLAGS) $(SHARED_OBJECTS) $(LIBS)
	cd lib && ln -s $(OUTPUT).so.$(VERSION) $(OUTPUT).so
	
makedir:
	$(MAKEDIR) lib
	$(MAKEDIR) obj/static/src
	$(MAKEDIR) obj/shared/src
	
test: test-client test-server
	
test-client:
	$(MAKE) -C ./test/nymph_test_client
	
test-server:
	$(MAKE) -C ./test/nymph_test_server

clean: clean-lib clean-test

clean-test: clean-test-client clean-test-server

clean-lib:
	$(RM) $(OBJECTS) $(SHARED_OBJECTS)
	
clean-test-client:
	$(MAKE) -C ./test/nymph_test_client clean
	
clean-test-server:
	$(MAKE) -C ./test/nymph_test_server clean
	
