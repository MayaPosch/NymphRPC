# Makefile for the NymphRC library.
#
# Allows one to build the library as a .a file, or
# build the test server and client.
#
# (c) 2017 Nyanko.ws

export TOP := $(CURDIR)

ifndef ANDROID_ABI_LEVEL
ANDROID_ABI_LEVEL := 21
endif

ifdef ANDROID
TOOLCHAIN_PREFIX := arm-linux-androideabi-
ARCH := android-armv7/
ifdef OS
TOOLCHAIN_POSTFIX := .cmd
endif
else ifdef ANDROID64
TOOLCHAIN_PREFIX := aarch64-linux-android-
ARCH := android-aarch64/
ifdef OS
TOOLCHAIN_POSTFIX := .cmd
endif
else ifdef ANDROIDX86
TOOLCHAIN_PREFIX := i686-linux-android-
ARCH := android-i686/
ifdef OS
TOOLCHAIN_POSTFIX := .cmd
endif
endif

ifndef ARCH
ARCH := $(shell g++ -dumpmachine)/
endif

USYS := $(shell uname -s)
UMCH := $(shell uname -m)

ifdef ANDROID
#GCC := $(TOOLCHAIN_PREFIX)g++$(TOOLCHAIN_POSTFIX)
GCC := armv7a-linux-androideabi$(ANDROID_ABI_LEVEL)-clang++$(TOOLCHAIN_POSTFIX)
MAKEDIR = mkdir -p
RM = rm
AR = $(TOOLCHAIN_PREFIX)ar
else ifdef ANDROID64
GCC := aarch64-linux-android$(ANDROID_ABI_LEVEL)-clang++$(TOOLCHAIN_POSTFIX)
MAKEDIR = mkdir -p
RM = rm
AR = $(TOOLCHAIN_PREFIX)ar
else ifdef ANDROIDX86
GCC := i686-linux-android$(ANDROID_ABI_LEVEL)-clang++$(TOOLCHAIN_POSTFIX)
MAKEDIR = mkdir -p
RM = rm
AR = $(TOOLCHAIN_PREFIX)ar
else ifdef WASM
GCC = emc++
MAKEDIR = mkdir -p
RM = rm
AR = ar 
else
GCC = g++
MAKEDIR = mkdir -p
RM = rm
AR = ar
endif


OUTPUT = libnymphrpc
VERSION = 0.1


# Use -soname on Linux/BSD, -install_name on Darwin (MacOS).
SONAME = -soname
LIBNAME = $(OUTPUT).so.$(VERSION)
ifeq ($(shell uname -s),Darwin)
	SONAME = -install_name
	LIBNAME = $(OUTPUT).0.dylib
endif


INCLUDE = -I src
LIBS := -lPocoNet -lPocoUtil -lPocoFoundation -lPocoJSON 
CFLAGS := $(INCLUDE) -g3 -std=c++11 -O0
SHARED_FLAGS := -fPIC -shared -Wl,$(SONAME),$(LIBNAME)

ifdef ANDROID
CFLAGS += -fPIC
else ifdef ANDROID64
#CFLAGS += -fPIC
else ifdef ANDROIDX86
CFLAGS += -fPIC
endif

# Check for MinGW and patch up POCO
# The OS variable is only set on Windows.
ifdef OS
ifndef ANDROID
ifndef ANDROID64
ifndef ANDROIDX86
	CFLAGS := $(CFLAGS) -U__STRICT_ANSI__ -DPOCO_WIN32_UTF8
	LIBS += -lws2_32
endif
endif
endif
else
	LIBS += -pthread
endif


SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(addprefix obj/static/$(ARCH),$(notdir) $(SOURCES:.cpp=.o))
SHARED_OBJECTS := $(addprefix obj/shared/$(ARCH),$(notdir) $(SOURCES:.cpp=.o))

all: lib

lib: makedir lib/$(ARCH)$(OUTPUT).a lib/$(ARCH)$(LIBNAME)
	
obj/static/$(ARCH)%.o: %.cpp
	$(GCC) -c -o $@ $< $(CFLAGS)
	
obj/shared/$(ARCH)%.o: %.cpp
	$(GCC) -c -o $@ $< $(SHARED_FLAGS) $(CFLAGS) $(LIBS)
	
lib/$(ARCH)$(OUTPUT).a: $(OBJECTS)
	-rm -f $@
	$(AR) rcs $@ $^
	
lib/$(ARCH)$(LIBNAME): $(SHARED_OBJECTS)
	$(GCC) -o $@ $(CFLAGS) $(SHARED_FLAGS) $(SHARED_OBJECTS) $(LIBS)
	
makedir:
	$(MAKEDIR) lib/$(ARCH)
	$(MAKEDIR) obj/static/$(ARCH)src
	$(MAKEDIR) obj/shared/$(ARCH)src
	
test: test-client test-server
	
test-client: lib
	$(MAKE) -C ./test/nymph_test_client
	
test-server: lib
	$(MAKE) -C ./test/nymph_test_server

clean: clean-lib

clean-test: clean-test-client clean-test-server

clean-lib:
	$(RM) $(OBJECTS) $(SHARED_OBJECTS)
	
clean-test-client:
	$(MAKE) -C ./test/nymph_test_client clean
	
clean-test-server:
	$(MAKE) -C ./test/nymph_test_server clean

PREFIX ?= /usr
ifdef OS
# Assume 64-bit MSYS2
PREFIX = /mingw64
endif

.PHONY: install
install:
	install -d $(DESTDIR)$(PREFIX)/lib/
	install -m 644 lib/$(ARCH)$(OUTPUT).a $(DESTDIR)$(PREFIX)/lib/
ifndef OS
	install -m 644 lib/$(ARCH)$(OUTPUT).so.$(VERSION) $(DESTDIR)$(PREFIX)/lib/
endif
	install -d $(DESTDIR)$(PREFIX)/include/nymph
	install -m 644 src/*.h $(DESTDIR)$(PREFIX)/include/nymph/
ifndef OS
	cd $(DESTDIR)$(PREFIX)/lib && \
		if [ -f $(OUTPUT).so ]; then \
			rm $(OUTPUT).so; \
		fi && \
		ln -s $(OUTPUT).so.$(VERSION) $(OUTPUT).so
endif

package:
	tar -C lib/$(ARCH) -cvzf lib/$(OUTPUT)-$(VERSION)-$(USYS)-$(UMCH).tar.gz $(OUTPUT).a $(OUTPUT).so.$(VERSION)
