rem @echo off

REM The olsr.org Optimized Link-State Routing daemon (olsrd)
REM
REM (c) by the OLSR project
REM
REM See our Git repository to find out who worked on this file
REM and thus is a copyright holder on it.
REM
REM All rights reserved.
REM
REM Redistribution and use in source and binary forms, with or without
REM modification, are permitted provided that the following conditions
REM are met:
REM
REM * Redistributions of source code must retain the above copyright
REM   notice, this list of conditions and the following disclaimer.
REM * Redistributions in binary form must reproduce the above copyright
REM   notice, this list of conditions and the following disclaimer in
REM   the documentation and/or other materials provided with the
REM   distribution.
REM * Neither the name of olsr.org, olsrd nor the names of its
REM   contributors may be used to endorse or promote products derived
REM   from this software without specific prior written permission.
REM
REM THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
REM "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
REM LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
REM FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
REM COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
REM INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
REM BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
REM LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
REM CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
REM LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
REM ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
REM POSSIBILITY OF SUCH DAMAGE.
REM
REM Visit http://www.olsr.org for more information.
REM
REM If you find this software useful feel free to make a donation
REM to the project. For more information see the website or contact
REM the copyright holders.

REM "make win32setup" does not work under Wine+Cygwin
REM Compile with this: "wine cmd /c make/makewine.cmd"

SET PRGS=C:\Programme
SET PSDK=%PRGS%\Microsoft SDK
SET MSVC=%PRGS%\Microsoft Visual Studio
SET PATH=%MSVC%\VC98\BIN;%PSDK%\BIN;%PRGS%\NSIS;%PATH%
SET LIB=%PSDK%\LIB;%MSVC%\VC98\LIB;%MSVC%\VC98\MFC\LIB
SET INCLUDE=%PSDK%\INCLUDE;%MSVC%\VC98\INCLUDE;%MSVC%\VC98\MFC\INCLUDE

IF NOT EXIST gui\win32\Shim\Release\*.* MKDIR gui\win32\Shim\Release

CL.EXE /nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"gui\win32\Shim\Release\Shim.pch" /YX /Fo"gui\win32\Shim\Release\\" /Fd"gui\win32\Shim\Release\\" /FD /c "gui\win32\Shim\shim.c"
LINK.EXE kernel32.lib user32.lib /nologo /subsystem:console /incremental:no /machine:I386 /out:"gui\win32\Shim\Release\Shim.exe" "gui\win32\Shim\Release\shim.obj"

IF NOT EXIST gui\win32\Main\Release\*.* MKDIR gui\win32\Main\Release

RC.EXE /l 0x407 /fo"gui\win32\Main\Release\Frontend.res" /d NDEBUG "gui\win32\Main\Frontend.rc"
CL.EXE /I"src" /nologo /MT /W4 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"gui\win32\Main\Release\Frontend.pch" /YX"stdafx.h" /Fo"gui\win32\Main\Release\\" /Fd"gui\win32\Main\Release\\" /FD /c "gui\win32\Main\Frontend.cpp" "gui\win32\Main\FrontendDlg.cpp" "gui\win32\Main\HnaEntry.cpp" "gui\win32\Main\MidEntry.cpp" "gui\win32\Main\MprEntry.cpp" "gui\win32\Main\MyDialog1.cpp" "gui\win32\Main\MyDialog2.cpp" "gui\win32\Main\MyDialog3.cpp" "gui\win32\Main\MyDialog4.cpp" "gui\win32\Main\MyEdit.cpp" "gui\win32\Main\MyTabCtrl.cpp" "gui\win32\Main\NodeEntry.cpp" "gui\win32\Main\StdAfx.cpp" "gui\win32\Main\TrayIcon.cpp"
LINK.EXE ws2_32.lib iphlpapi.lib gui/win32/Main/olsrd_cfgparser.lib gui/win32/Main/Release/Frontend.res /nologo /subsystem:windows /incremental:no /machine:I386 /out:"gui\win32\Main\Release\Switch.exe" "gui\win32\Main\Release\Frontend.obj" "gui\win32\Main\Release\FrontendDlg.obj" "gui\win32\Main\Release\HnaEntry.obj" "gui\win32\Main\Release\MidEntry.obj" "gui\win32\Main\Release\MprEntry.obj" "gui\win32\Main\Release\MyDialog1.obj" "gui\win32\Main\Release\MyDialog2.obj" "gui\win32\Main\Release\MyDialog3.obj" "gui\win32\Main\Release\MyDialog4.obj" "gui\win32\Main\Release\MyEdit.obj" "gui\win32\Main\Release\MyTabCtrl.obj" "gui\win32\Main\Release\NodeEntry.obj" "gui\win32\Main\Release\StdAfx.obj" "gui\win32\Main\Release\TrayIcon.obj"

MAKENSIS.EXE gui\win32\Inst\installer.nsi
