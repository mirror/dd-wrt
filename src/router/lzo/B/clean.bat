@echo off

set RM=rm -f

%RM% *.err *.map *.o *.obj *.tds
%RM% src\i386\d_asm3\*.obj
%RM% liblzo.a lzo.dll lzo.exp lzo.lib
%RM% ltest.exe
%RM% dict.exe lpack.exe overlap.exe precomp.exe precomp2.exe simple.exe
%RM% align.exe chksum.exe
%RM% testmini.exe

