# NymphRPC #

**Author:** Maya Posch

**Date:** 2018/01/27

## Overview ##

NymphRPC is a compact, C++-based Remote Procedure Call (RPC) library. One can look at the example server and client implementations in the `test` folder to get an idea of how NymphRPC is integrated into an application.

## Planned ports ##

In addition to the current C++ implementation, an Ada version of the library is also planned.

## Building ##

NymphRPC's C++ version is based around the POCO library (see [https://pocoproject.org/](https://pocoproject.org/ "POCO Project") ). Version 1.5+ is required.

With the common C++ build tools installed (g++, make, etc.) and the optional OpenSSL dependency, building NymphRPC is done using:

`make` - Build just the library. Found under `lib`.This builds both a static and shared library.

`make test-client` - Build the test client.

`make test-server` - Build the test server.

`make test` - Build both the test client & server.

`make clean` - Remove the library's build files.

`make clean-lib` - Clean just the `lib` target.

`make clean-test` - Clean just the `test` target.

`make clean-test-client` - Clean just the test client.

`make clean-test-server` - Clean just the test server.

## Installing ##

Use `make install` to install NymphRPC after building with `make`. This has been tested with MSYS2 (Windows) and Linux.

Installing the resulting `libnymphrpc` library on other platforms may have to be done by hand, currently. On Linux one would copy the static library `libnymphrpc.a` and/or the `libnymphrpc.so` shared library file to `/usr/lib`, and the header files to `/usr/include` or similar. Consult the documentation for your OS and development tools for more detailed instructions.

