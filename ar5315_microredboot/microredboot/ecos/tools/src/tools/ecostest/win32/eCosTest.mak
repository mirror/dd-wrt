# Microsoft Developer Studio Generated NMAKE File, Based on eCosTest.dsp
!IF "$(CFG)" == ""
CFG=eCosTest - Win32 Debug
!MESSAGE No configuration specified. Defaulting to eCosTest - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "eCosTest - Win32 Release" && "$(CFG)" != "eCosTest - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "eCosTest.mak" CFG="eCosTest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "eCosTest - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "eCosTest - Win32 Debug" (based on "Win32 (x86) Static Library")
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

!IF  "$(CFG)" == "eCosTest - Win32 Release"

OUTDIR=v:\eCosTest\release
INTDIR=v:\eCosTest\release
# Begin Custom Macros
OutDir=v:\eCosTest\release
# End Custom Macros

ALL : "$(OUTDIR)\eCosTest.lib" "$(OUTDIR)\eCosTest.bsc"


CLEAN :
	-@erase "$(INTDIR)\eCosTest.obj"
	-@erase "$(INTDIR)\eCosTest.sbr"
	-@erase "$(INTDIR)\eCosTestDownloadFilter.obj"
	-@erase "$(INTDIR)\eCosTestDownloadFilter.sbr"
	-@erase "$(INTDIR)\eCosTestSerial.obj"
	-@erase "$(INTDIR)\eCosTestSerial.sbr"
	-@erase "$(INTDIR)\eCosTestSerialFilter.obj"
	-@erase "$(INTDIR)\eCosTestSerialFilter.sbr"
	-@erase "$(INTDIR)\eCosTestSocket.obj"
	-@erase "$(INTDIR)\eCosTestSocket.sbr"
	-@erase "$(INTDIR)\eCosTestUtils.obj"
	-@erase "$(INTDIR)\eCosTestUtils.sbr"
	-@erase "$(INTDIR)\Properties.obj"
	-@erase "$(INTDIR)\Properties.sbr"
	-@erase "$(INTDIR)\TestResource.obj"
	-@erase "$(INTDIR)\TestResource.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\X10.obj"
	-@erase "$(INTDIR)\X10.sbr"
	-@erase "$(OUTDIR)\eCosTest.bsc"
	-@erase "$(OUTDIR)\eCosTest.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W4 /GX /Zi /O2 /I "wsock32.lib" /I "..\common" /I "..\..\utils\common" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\eCosTest.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\eCosTest.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\eCosTest.sbr" \
	"$(INTDIR)\eCosTestDownloadFilter.sbr" \
	"$(INTDIR)\eCosTestSerial.sbr" \
	"$(INTDIR)\eCosTestSerialFilter.sbr" \
	"$(INTDIR)\eCosTestSocket.sbr" \
	"$(INTDIR)\eCosTestUtils.sbr" \
	"$(INTDIR)\Properties.sbr" \
	"$(INTDIR)\TestResource.sbr" \
	"$(INTDIR)\X10.sbr"

"$(OUTDIR)\eCosTest.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\eCosTest.lib" 
LIB32_OBJS= \
	"$(INTDIR)\eCosTest.obj" \
	"$(INTDIR)\eCosTestDownloadFilter.obj" \
	"$(INTDIR)\eCosTestSerial.obj" \
	"$(INTDIR)\eCosTestSerialFilter.obj" \
	"$(INTDIR)\eCosTestSocket.obj" \
	"$(INTDIR)\eCosTestUtils.obj" \
	"$(INTDIR)\Properties.obj" \
	"$(INTDIR)\TestResource.obj" \
	"$(INTDIR)\X10.obj"

"$(OUTDIR)\eCosTest.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "eCosTest - Win32 Debug"

OUTDIR=v:\eCosTest\debug
INTDIR=v:\eCosTest\debug
# Begin Custom Macros
OutDir=v:\eCosTest\debug
# End Custom Macros

ALL : "$(OUTDIR)\eCosTest.lib" "$(OUTDIR)\eCosTest.bsc"


CLEAN :
	-@erase "$(INTDIR)\eCosTest.obj"
	-@erase "$(INTDIR)\eCosTest.sbr"
	-@erase "$(INTDIR)\eCosTestDownloadFilter.obj"
	-@erase "$(INTDIR)\eCosTestDownloadFilter.sbr"
	-@erase "$(INTDIR)\eCosTestSerial.obj"
	-@erase "$(INTDIR)\eCosTestSerial.sbr"
	-@erase "$(INTDIR)\eCosTestSerialFilter.obj"
	-@erase "$(INTDIR)\eCosTestSerialFilter.sbr"
	-@erase "$(INTDIR)\eCosTestSocket.obj"
	-@erase "$(INTDIR)\eCosTestSocket.sbr"
	-@erase "$(INTDIR)\eCosTestUtils.obj"
	-@erase "$(INTDIR)\eCosTestUtils.sbr"
	-@erase "$(INTDIR)\Properties.obj"
	-@erase "$(INTDIR)\Properties.sbr"
	-@erase "$(INTDIR)\TestResource.obj"
	-@erase "$(INTDIR)\TestResource.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\X10.obj"
	-@erase "$(INTDIR)\X10.sbr"
	-@erase "$(OUTDIR)\eCosTest.bsc"
	-@erase "$(OUTDIR)\eCosTest.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W4 /Gm /GX /ZI /Od /I "..\common" /I "..\..\utils\common" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\eCosTest.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\eCosTest.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\eCosTest.sbr" \
	"$(INTDIR)\eCosTestDownloadFilter.sbr" \
	"$(INTDIR)\eCosTestSerial.sbr" \
	"$(INTDIR)\eCosTestSerialFilter.sbr" \
	"$(INTDIR)\eCosTestSocket.sbr" \
	"$(INTDIR)\eCosTestUtils.sbr" \
	"$(INTDIR)\Properties.sbr" \
	"$(INTDIR)\TestResource.sbr" \
	"$(INTDIR)\X10.sbr"

"$(OUTDIR)\eCosTest.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\eCosTest.lib" 
LIB32_OBJS= \
	"$(INTDIR)\eCosTest.obj" \
	"$(INTDIR)\eCosTestDownloadFilter.obj" \
	"$(INTDIR)\eCosTestSerial.obj" \
	"$(INTDIR)\eCosTestSerialFilter.obj" \
	"$(INTDIR)\eCosTestSocket.obj" \
	"$(INTDIR)\eCosTestUtils.obj" \
	"$(INTDIR)\Properties.obj" \
	"$(INTDIR)\TestResource.obj" \
	"$(INTDIR)\X10.obj"

"$(OUTDIR)\eCosTest.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
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
!IF EXISTS("eCosTest.dep")
!INCLUDE "eCosTest.dep"
!ELSE 
!MESSAGE Warning: cannot find "eCosTest.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "eCosTest - Win32 Release" || "$(CFG)" == "eCosTest - Win32 Debug"
SOURCE=..\common\eCosTest.cpp

"$(INTDIR)\eCosTest.obj"	"$(INTDIR)\eCosTest.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\common\eCosTestDownloadFilter.cpp

"$(INTDIR)\eCosTestDownloadFilter.obj"	"$(INTDIR)\eCosTestDownloadFilter.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\common\eCosTestSerial.cpp

"$(INTDIR)\eCosTestSerial.obj"	"$(INTDIR)\eCosTestSerial.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\common\eCosTestSerialFilter.cpp

"$(INTDIR)\eCosTestSerialFilter.obj"	"$(INTDIR)\eCosTestSerialFilter.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\common\eCosTestSocket.cpp

"$(INTDIR)\eCosTestSocket.obj"	"$(INTDIR)\eCosTestSocket.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\common\eCosTestUtils.cpp

"$(INTDIR)\eCosTestUtils.obj"	"$(INTDIR)\eCosTestUtils.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\Utils\common\Properties.cpp

"$(INTDIR)\Properties.obj"	"$(INTDIR)\Properties.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\common\TestResource.cpp

"$(INTDIR)\TestResource.obj"	"$(INTDIR)\TestResource.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\common\X10.cpp

"$(INTDIR)\X10.obj"	"$(INTDIR)\X10.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

