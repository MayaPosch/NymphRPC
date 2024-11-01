# Makefile for the NymphRC library.
#
# Allows one to build the library as a .a file, or
# build the test server and client.
#
# (c) 2017 Nyanko.ws

export TOP := $(CURDIR)

ifndef ANDROID_ABI_LEVEL
ANDROID_ABI_LEVEL := 24
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
else ifdef ANDROIDX64
TOOLCHAIN_PREFIX := x86_64-linux-android-
ARCH := android-x86_64/
ifdef OS
TOOLCHAIN_POSTFIX := .cmd
endif
endif

ifndef ARCH
ARCH := $(shell g++ -dumpmachine)/
endif

USYS := $(shell uname -s)
UMCH := $(shell uname -m)

ifdef TOOLCHAIN
include toolchain/$(TOOLCHAIN).mk
else
GPP = g++
GCC = gcc
STRIP = strip
MAKEDIR = mkdir -p
RM = rm
endif

ifdef ANDROID
#GPP := $(TOOLCHAIN_PREFIX)g++$(TOOLCHAIN_POSTFIX)
GPP := armv7a-linux-androideabi$(ANDROID_ABI_LEVEL)-clang++$(TOOLCHAIN_POSTFIX)
MAKEDIR = mkdir -p
RM = rm
AR = $(TOOLCHAIN_PREFIX)ar
else ifdef ANDROID64
GPP := aarch64-linux-android$(ANDROID_ABI_LEVEL)-clang++$(TOOLCHAIN_POSTFIX)
MAKEDIR = mkdir -p
RM = rm
AR = $(TOOLCHAIN_PREFIX)ar
else ifdef ANDROIDX86
GPP := i686-linux-android$(ANDROID_ABI_LEVEL)-clang++$(TOOLCHAIN_POSTFIX)
MAKEDIR = mkdir -p
RM = rm
AR = $(TOOLCHAIN_PREFIX)ar
else ifdef ANDROIDX64
GPP := x86_64-linux-android$(ANDROID_ABI_LEVEL)-clang++$(TOOLCHAIN_POSTFIX)
MAKEDIR = mkdir -p
RM = rm
AR = $(TOOLCHAIN_PREFIX)ar
else ifdef WASM
GPP = emc++
MAKEDIR = mkdir -p
RM = rm
AR = ar 
else
GPP ?= g++
MAKEDIR ?= mkdir -p
RM ?= rm
AR ?= ar
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
ifdef NPOCO
LIBS := -lNPocoNet -lNPocoCore
CFLAGS := $(CFLAGS) -DNPOCO
else
LIBS := -lPocoNet -lPocoUtil -lPocoFoundation -lPocoJSON 
endif

ifeq ($(USYS),FreeBSD)
	INCLUDE += -I /usr/local/include
	LIBS += -L /usr/local/lib
endif

CFLAGS := $(INCLUDE) $(CFLAGS) -g3 -std=c++14 -O0
SHARED_FLAGS := -fPIC -shared -Wl,$(SONAME),$(LIBNAME)

ifeq ($(GPP),g++)
	CFLAGS += -fext-numeric-literals
endif

ifdef ANDROID
CFLAGS += -fPIC
else ifdef ANDROID64
CFLAGS += -fPIC
else ifdef ANDROIDX86
CFLAGS += -fPIC
else ifdef ANDROIDX64
CFLAGS += -fPIC
endif

# Check for MinGW and patch up POCO
# The OS variable is only set on Windows.
ifdef OS
ifndef ANDROID
ifndef ANDROID64
ifndef ANDROIDX86
ifndef ANDROIDX64
	# -U__STRICT_ANSI__
	CFLAGS := $(CFLAGS) -DPOCO_WIN32_UTF8
	LIBS += -lws2_32
endif
endif
endif
endif
else
	LIBS += -pthread
	UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        #MAKE := gmake
		# Should be BSD?
    endif
endif


SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(addprefix obj/static/$(ARCH),$(notdir) $(SOURCES:.cpp=.o))
SHARED_OBJECTS := $(addprefix obj/shared/$(ARCH),$(notdir) $(SOURCES:.cpp=.o))

all: lib

lib: makedir lib/$(ARCH)$(OUTPUT).a lib/$(ARCH)$(LIBNAME)
	
obj/static/$(ARCH)%.o: %.cpp
	$(GPP) -c -o $@ $< $(CFLAGS)
	
obj/shared/$(ARCH)%.o: %.cpp
	$(GPP) -c -o $@ $< $(SHARED_FLAGS) $(CFLAGS) $(LIBS)
	
lib/$(ARCH)$(OUTPUT).a: $(OBJECTS)
	-rm -f $@
	$(AR) rcs $@ $^
	
lib/$(ARCH)$(LIBNAME): $(SHARED_OBJECTS)
	$(GPP) -o $@ $(CFLAGS) $(SHARED_FLAGS) $(SHARED_OBJECTS) $(LIBS)
	
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
#PREFIX = /mingw64
PREFIX = $(MINGW_PREFIX)
endif

ifeq ($(USYS),Haiku)
	PREFIX := /boot/system/non-packaged
	DEVFOLDER := /develop
	HAIKU := true
endif

.PHONY: install
install:
	install -d $(DESTDIR)$(PREFIX)/lib/
	install -m 644 lib/$(ARCH)$(OUTPUT).a $(DESTDIR)$(PREFIX)$(DEVFOLDER)/lib/
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
