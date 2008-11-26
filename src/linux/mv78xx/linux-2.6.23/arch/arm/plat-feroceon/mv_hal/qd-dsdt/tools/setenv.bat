@echo off
set REQ_OS=%1

if "%REQ_OS%"==""        set OS_RUN=VXWORKS
if "%REQ_OS%"=="VXWORKS" set OS_RUN=VXWORKS
if "%REQ_OS%"=="vxworks" set OS_RUN=VXWORKS
if "%REQ_OS%"=="vxWorks" set OS_RUN=VXWORKS
if "%REQ_OS%"=="win32"   set OS_RUN=WIN32
if "%REQ_OS%"=="WIN32"   set OS_RUN=WIN32
if "%REQ_OS%"=="wince"   set OS_RUN=WIN32
if "%REQ_OS%"=="WINCE"   set OS_RUN=WIN32

set USER_BASE=C:\DSDT_2.7
set PROJ_NAME=qdDrv
set TOOL_DIR=%USER_BASE%\tools
set RELEASE=YES
set TARGET_CPU=ARM
rem set TARGET_CPU=MIPS

if "%OS_RUN%"=="VXWORKS" goto VXWORKS_ENV
if "%OS_RUN%"=="WIN32" goto WIN32_ENV

echo Unknown Target OS!
echo Supported Target OS is vxworks and wince.
echo Assumes VxWorks as a Target OS.

:VXWORKS_ENV

set TOR_VER=2.1
set ENDIAN=EL
set WIND_BASE=C:\Tornado.arm
rem set WIND_BASE=C:\Tornado2.1
set WIND_HOST_TYPE=x86-win32

set PATH=%WIND_BASE%\host\%WIND_HOST_TYPE%\bin;%PATH%

echo Environment Variable for VxWorks has been set.

goto DONE

:WIN32_ENV

set TARGETCPU=MIPSIV
rem set TARGETCPU=x86
set WCEROOT=C:\WINCE400

set INCLUDE=%WCEROOT%\public\common\oak\inc;%WCEROOT%\public\common\sdk\inc;%WCEROOT%\public\common\ddk\inc;%INCLUDE%
set PATH=%WCEROOT%\sdk\bin\i386;%WCEROOT%\public\common\oak\bin\i386;%USER_BASE%\tools;%path%

echo Environment Variable for WinCE has been set.
goto DONE

:DONE
cd %USER_BASE%\src

