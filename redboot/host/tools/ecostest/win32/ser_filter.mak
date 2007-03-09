# Microsoft Developer Studio Generated NMAKE File, Based on ser_filter.dsp
!IF "$(CFG)" == ""
CFG=ser_filter - Win32 Debug
!MESSAGE No configuration specified. Defaulting to ser_filter - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "ser_filter - Win32 Release" && "$(CFG)" != "ser_filter - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ser_filter.mak" CFG="ser_filter - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ser_filter - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "ser_filter - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ser_filter - Win32 Release"

OUTDIR=v:\eCosTest\release
INTDIR=v:\eCosTest\release
# Begin Custom Macros
OutDir=v:\eCosTest\release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\ser_filter.exe" "$(OUTDIR)\ser_filter.bsc"

!ELSE 

ALL : "eCosTest - Win32 Release" "$(OUTDIR)\ser_filter.exe" "$(OUTDIR)\ser_filter.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"eCosTest - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\ser_filter.obj"
	-@erase "$(INTDIR)\ser_filter.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\ser_filter.bsc"
	-@erase "$(OUTDIR)\ser_filter.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W4 /GX /Zd /O2 /I "..\..\utils\common" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\ser_filter.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ser_filter.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\ser_filter.sbr"

"$(OUTDIR)\ser_filter.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\ser_filter.pdb" /machine:I386 /out:"$(OUTDIR)\ser_filter.exe" /swaprun:net 
LINK32_OBJS= \
	"$(INTDIR)\ser_filter.obj" \
	".\eCosTest___Win32_Release\eCosTest.lib"

"$(OUTDIR)\ser_filter.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ser_filter - Win32 Debug"

OUTDIR=v:\eCosTest\debug
INTDIR=v:\eCosTest\debug
# Begin Custom Macros
OutDir=v:\eCosTest\debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\ser_filter.exe" "$(OUTDIR)\ser_filter.bsc"

!ELSE 

ALL : "eCosTest - Win32 Debug" "$(OUTDIR)\ser_filter.exe" "$(OUTDIR)\ser_filter.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"eCosTest - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\ser_filter.obj"
	-@erase "$(INTDIR)\ser_filter.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\ser_filter.bsc"
	-@erase "$(OUTDIR)\ser_filter.exe"
	-@erase "$(OUTDIR)\ser_filter.ilk"
	-@erase "$(OUTDIR)\ser_filter.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W4 /Gm /GX /ZI /Od /I "..\..\utils\common" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ser_filter.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\ser_filter.sbr"

"$(OUTDIR)\ser_filter.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\ser_filter.pdb" /debug /machine:I386 /out:"$(OUTDIR)\ser_filter.exe" /pdbtype:sept /swaprun:net 
LINK32_OBJS= \
	"$(INTDIR)\ser_filter.obj" \
	"$(OUTDIR)\eCosTest.lib"

"$(OUTDIR)\ser_filter.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("ser_filter.dep")
!INCLUDE "ser_filter.dep"
!ELSE 
!MESSAGE Warning: cannot find "ser_filter.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "ser_filter - Win32 Release" || "$(CFG)" == "ser_filter - Win32 Debug"
SOURCE=..\common\ser_filter.cpp

"$(INTDIR)\ser_filter.obj"	"$(INTDIR)\ser_filter.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!IF  "$(CFG)" == "ser_filter - Win32 Release"

"eCosTest - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F .\eCosTest.mak CFG="eCosTest - Win32 Release" 
   cd "."

"eCosTest - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F .\eCosTest.mak CFG="eCosTest - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "ser_filter - Win32 Debug"

"eCosTest - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F .\eCosTest.mak CFG="eCosTest - Win32 Debug" 
   cd "."

"eCosTest - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F .\eCosTest.mak CFG="eCosTest - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 


!ENDIF 

