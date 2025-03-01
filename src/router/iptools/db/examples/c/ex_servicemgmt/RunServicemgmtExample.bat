@ECHO OFF

IF EXIST ..\..\..\build_windows\Win32\Debug\ServicemgmtExample rmdir ..\..\..\build_windows\Win32\Debug\ServicemgmtExample /q/s

MKDIR ..\..\..\build_windows\Win32\Debug\ServicemgmtExample

XCOPY customer.txt ..\..\..\build_windows\Win32\Debug\ServicemgmtExample

XCOPY engineer.txt ..\..\..\build_windows\Win32\Debug\ServicemgmtExample

XCOPY service.txt ..\..\..\build_windows\Win32\Debug\ServicemgmtExample

ECHO Starting the servicemgmt project.

CD ..\..\..\build_windows\Win32\Debug

START "" .\ex_servicemgmt.exe
