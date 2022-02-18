@echo off & setlocal enableextensions enabledelayedexpansion
::
:: Setup prerequisites and build NymphRPC library (MSVC).
::
:: Created 23 January 2022.
:: Copyright (c) 2021 Nyanko.ws
::
:: Usage: Setup-NMake-vcpkg [POCO_ROOT=path/to/lib] [target]
::

:: Install vcpkg tool:
:: > git clone https://github.com/microsoft/vcpkg
:: > .\vcpkg\bootstrap-vcpkg.bat -disableMetrics
::

echo.

set INSTALL_PREFIX=D:\Libraries\NymphRPC

:: Note: static building does not yet work.
set NC_STATIC=0
:: set NC_STATIC=1

set NC_CONFIG=Release
:: set NC_CONFIG=Debug

set NC_TGT_BITS=64
set NC_TGT_ARCH=x%NC_TGT_BITS%

set NC_LNKCRT=-MD
set VCPKG_TRIPLET=x64-windows

if [%NC_STATIC%] == [1] (
    set NC_LNKCRT=-MT
    set VCPKG_TRIPLET=x64-windows-static
    echo [Setup NymphRPC: static build does not yet work. Continuing.]
)

:: Check for 64-bit Native Tools Command Prompt

if not [%VSCMD_ARG_TGT_ARCH%] == [%NC_TGT_ARCH%] (
    echo [Setup NymphRPC: Make sure to run these commands in a '%NC_TGT_BITS%-bit Native Tools Command Prompt'; expecting 'NC_TGT_ARCH', got '%VSCMD_ARG_TGT_ARCH%'. Bailing out.]
    endlocal & goto :EOF
)

if [%VCPKG_ROOT%] == [] (
    echo [Setup NymphRPC: Make sure environment variable 'VCPKG_ROOT' points to your vcpkg installation; it's empty or does not exist. Bailing out.]
    endlocal & goto :EOF
)

:: Make sure NymphRPC and LibNymphCast will be build with the same Poco version:

if exist "%VCPKG_ROOT%\installed\%VCPKG_TRIPLET%\include\Poco" (
    echo Setup NymphRPC: Poco is already installed at "%VCPKG_ROOT%\installed\%VCPKG_TRIPLET%\include\Poco".
) else (
    echo [Installing vcpkg Poco; please be patient, this may take about 10 minutes...]
    vcpkg install --triplet %VCPKG_TRIPLET% poco
)

echo Setup NymphRPC: Using POCO_ROOT=%VCPKG_ROOT%\installed\%VCPKG_TRIPLET%

set POCO_ROOT=%VCPKG_ROOT%\installed\%VCPKG_TRIPLET%

:: Make NymphRPC.lib and NymphRPCmt.lib:

::set CMD_ARGS=%*
::if [%CMD_ARGS%] == [] (
::  set CMD_ARGS=clean all install
::)
::echo CMD_ARGS: '%CMD_ARGS%'

nmake -nologo -f NMakefile ^
       NC_STATIC=%NC_STATIC% ^
       NC_LNKCRT=-MD ^
       NC_CONFIG=%NC_CONFIG% ^
       POCO_ROOT=%POCO_ROOT% ^
  INSTALL_PREFIX=%INSTALL_PREFIX% ^
        clean all install %*

nmake -nologo -f NMakefile ^
       NC_STATIC=%NC_STATIC% ^
       NC_LNKCRT=-MT ^
       NC_CONFIG=%NC_CONFIG% ^
       POCO_ROOT=%POCO_ROOT% ^
  INSTALL_PREFIX=%INSTALL_PREFIX% ^
        clean all install %*

echo.

endlocal

:: End of file
