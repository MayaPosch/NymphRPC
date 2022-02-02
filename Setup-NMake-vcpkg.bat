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

::set NC_LNKCRT=-MT

set NC_TGT_BITS=64
set NC_TGT_ARCH=x%NC_TGT_BITS%

:: Check for 64-bit Native Tools Command Prompt

if not [%VSCMD_ARG_TGT_ARCH%] == [%NC_TGT_ARCH%] (
    echo [Make sure to run these commands in a '%NC_TGT_BITS%-bit Native Tools Command Prompt'; expecting 'NC_TGT_ARCH', got '%VSCMD_ARG_TGT_ARCH%'. Bailing out.]
    endlocal & goto :EOF
)

if [%VCPKG_ROOT%] == [] (
    echo [Make sure to environment variable 'VCPKG_ROOT' point to you vcpkg installation; it's empty or does not exist. Bailing out.]
    endlocal & goto :EOF
)

set VCPKG_TRIPLET=%NC_TGT_ARCH%-windows
::set VCPKG_TRIPLET=%NC_TGT_ARCH%-windows-static

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
    NC_LNKCRT=-MD ^
    POCO_ROOT=%POCO_ROOT% ^
        clean all install %*

nmake -nologo -f NMakefile ^
    NC_LNKCRT=-MT ^
    POCO_ROOT=%POCO_ROOT% ^
        clean all install %*

echo.

endlocal

:: End of file
