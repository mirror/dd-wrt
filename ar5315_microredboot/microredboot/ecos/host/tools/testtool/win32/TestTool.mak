# Microsoft Developer Studio Generated NMAKE File, Based on TestTool.dsp
!IF "$(CFG)" == ""
CFG=TestTool - Win32 Debug
!MESSAGE No configuration specified. Defaulting to TestTool - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "TestTool - Win32 Release" && "$(CFG)" != "TestTool - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TestTool.mak" CFG="TestTool - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TestTool - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "TestTool - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TestTool - Win32 Release"

OUTDIR=v:\eCosTest\Release
INTDIR=v:\eCosTest\TestTool\Release
# Begin Custom Macros
OutDir=v:\eCosTest\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\TestTool.exe" "$(OUTDIR)\TestTool.bsc"

!ELSE 

ALL : "eCosTest - Win32 Release" "$(OUTDIR)\TestTool.exe" "$(OUTDIR)\TestTool.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"eCosTest - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\ExecutionPage.obj"
	-@erase "$(INTDIR)\ExecutionPage.sbr"
	-@erase "$(INTDIR)\FileListBox.obj"
	-@erase "$(INTDIR)\FileListBox.sbr"
	-@erase "$(INTDIR)\LocalPropertiesDialog.obj"
	-@erase "$(INTDIR)\LocalPropertiesDialog.sbr"
	-@erase "$(INTDIR)\OutputEdit.obj"
	-@erase "$(INTDIR)\OutputEdit.sbr"
	-@erase "$(INTDIR)\OutputPage.obj"
	-@erase "$(INTDIR)\OutputPage.sbr"
	-@erase "$(INTDIR)\PropertiesDialog.obj"
	-@erase "$(INTDIR)\PropertiesDialog.sbr"
	-@erase "$(INTDIR)\RemotePropertiesDialog.obj"
	-@erase "$(INTDIR)\RemotePropertiesDialog.sbr"
	-@erase "$(INTDIR)\RunTestsDlg.obj"
	-@erase "$(INTDIR)\RunTestsDlg.sbr"
	-@erase "$(INTDIR)\RunTestsSheet.obj"
	-@erase "$(INTDIR)\RunTestsSheet.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\SummaryPage.obj"
	-@erase "$(INTDIR)\SummaryPage.sbr"
	-@erase "$(INTDIR)\TestTool.obj"
	-@erase "$(INTDIR)\TestTool.res"
	-@erase "$(INTDIR)\TestTool.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\TestTool.bsc"
	-@erase "$(OUTDIR)\TestTool.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP_PROJ=/nologo /MD /W4 /GX /O2 /I "..\TestSheet" /I "." /I "..\..\ecostest\common" /I "..\common" /I "..\..\utils\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\TestTool.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\TestTool.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\ExecutionPage.sbr" \
	"$(INTDIR)\FileListBox.sbr" \
	"$(INTDIR)\LocalPropertiesDialog.sbr" \
	"$(INTDIR)\OutputEdit.sbr" \
	"$(INTDIR)\OutputPage.sbr" \
	"$(INTDIR)\PropertiesDialog.sbr" \
	"$(INTDIR)\RemotePropertiesDialog.sbr" \
	"$(INTDIR)\RunTestsDlg.sbr" \
	"$(INTDIR)\RunTestsSheet.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\SummaryPage.sbr" \
	"$(INTDIR)\TestTool.sbr"

"$(OUTDIR)\TestTool.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=wsock32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\TestTool.pdb" /machine:I386 /out:"$(OUTDIR)\TestTool.exe" /swaprun:net 
LINK32_OBJS= \
	"$(INTDIR)\ExecutionPage.obj" \
	"$(INTDIR)\FileListBox.obj" \
	"$(INTDIR)\LocalPropertiesDialog.obj" \
	"$(INTDIR)\OutputEdit.obj" \
	"$(INTDIR)\OutputPage.obj" \
	"$(INTDIR)\PropertiesDialog.obj" \
	"$(INTDIR)\RemotePropertiesDialog.obj" \
	"$(INTDIR)\RunTestsDlg.obj" \
	"$(INTDIR)\RunTestsSheet.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\SummaryPage.obj" \
	"$(INTDIR)\TestTool.obj" \
	"$(INTDIR)\TestTool.res" \
	"..\..\ecostest\win32\eCosTest___Win32_Release\eCosTest.lib"

"$(OUTDIR)\TestTool.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "TestTool - Win32 Debug"

OUTDIR=v:\eCosTest\Debug
INTDIR=v:\eCosTest\TestTool\Debug
# Begin Custom Macros
OutDir=v:\eCosTest\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\TestTool.exe" "$(OUTDIR)\TestTool.bsc"

!ELSE 

ALL : "eCosTest - Win32 Debug" "$(OUTDIR)\TestTool.exe" "$(OUTDIR)\TestTool.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"eCosTest - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\ExecutionPage.obj"
	-@erase "$(INTDIR)\ExecutionPage.sbr"
	-@erase "$(INTDIR)\FileListBox.obj"
	-@erase "$(INTDIR)\FileListBox.sbr"
	-@erase "$(INTDIR)\LocalPropertiesDialog.obj"
	-@erase "$(INTDIR)\LocalPropertiesDialog.sbr"
	-@erase "$(INTDIR)\OutputEdit.obj"
	-@erase "$(INTDIR)\OutputEdit.sbr"
	-@erase "$(INTDIR)\OutputPage.obj"
	-@erase "$(INTDIR)\OutputPage.sbr"
	-@erase "$(INTDIR)\PropertiesDialog.obj"
	-@erase "$(INTDIR)\PropertiesDialog.sbr"
	-@erase "$(INTDIR)\RemotePropertiesDialog.obj"
	-@erase "$(INTDIR)\RemotePropertiesDialog.sbr"
	-@erase "$(INTDIR)\RunTestsDlg.obj"
	-@erase "$(INTDIR)\RunTestsDlg.sbr"
	-@erase "$(INTDIR)\RunTestsSheet.obj"
	-@erase "$(INTDIR)\RunTestsSheet.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\SummaryPage.obj"
	-@erase "$(INTDIR)\SummaryPage.sbr"
	-@erase "$(INTDIR)\TestTool.obj"
	-@erase "$(INTDIR)\TestTool.res"
	-@erase "$(INTDIR)\TestTool.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\TestTool.bsc"
	-@erase "$(OUTDIR)\TestTool.exe"
	-@erase "$(OUTDIR)\TestTool.ilk"
	-@erase "$(OUTDIR)\TestTool.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP_PROJ=/nologo /MDd /W4 /Gm /GX /ZI /Od /I "." /I "..\..\ecostest\common" /I "..\common" /I "..\..\utils\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\TestTool.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\TestTool.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\ExecutionPage.sbr" \
	"$(INTDIR)\FileListBox.sbr" \
	"$(INTDIR)\LocalPropertiesDialog.sbr" \
	"$(INTDIR)\OutputEdit.sbr" \
	"$(INTDIR)\OutputPage.sbr" \
	"$(INTDIR)\PropertiesDialog.sbr" \
	"$(INTDIR)\RemotePropertiesDialog.sbr" \
	"$(INTDIR)\RunTestsDlg.sbr" \
	"$(INTDIR)\RunTestsSheet.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\SummaryPage.sbr" \
	"$(INTDIR)\TestTool.sbr"

"$(OUTDIR)\TestTool.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=wsock32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\TestTool.pdb" /debug /machine:I386 /out:"$(OUTDIR)\TestTool.exe" /pdbtype:sept /swaprun:net 
LINK32_OBJS= \
	"$(INTDIR)\ExecutionPage.obj" \
	"$(INTDIR)\FileListBox.obj" \
	"$(INTDIR)\LocalPropertiesDialog.obj" \
	"$(INTDIR)\OutputEdit.obj" \
	"$(INTDIR)\OutputPage.obj" \
	"$(INTDIR)\PropertiesDialog.obj" \
	"$(INTDIR)\RemotePropertiesDialog.obj" \
	"$(INTDIR)\RunTestsDlg.obj" \
	"$(INTDIR)\RunTestsSheet.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\SummaryPage.obj" \
	"$(INTDIR)\TestTool.obj" \
	"$(INTDIR)\TestTool.res" \
	"$(OUTDIR)\eCosTest.lib"

"$(OUTDIR)\TestTool.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("TestTool.dep")
!INCLUDE "TestTool.dep"
!ELSE 
!MESSAGE Warning: cannot find "TestTool.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "TestTool - Win32 Release" || "$(CFG)" == "TestTool - Win32 Debug"
SOURCE=.\ExecutionPage.cpp

"$(INTDIR)\ExecutionPage.obj"	"$(INTDIR)\ExecutionPage.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\FileListBox.cpp

"$(INTDIR)\FileListBox.obj"	"$(INTDIR)\FileListBox.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\LocalPropertiesDialog.cpp

"$(INTDIR)\LocalPropertiesDialog.obj"	"$(INTDIR)\LocalPropertiesDialog.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\OutputEdit.cpp

"$(INTDIR)\OutputEdit.obj"	"$(INTDIR)\OutputEdit.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\OutputPage.cpp

"$(INTDIR)\OutputPage.obj"	"$(INTDIR)\OutputPage.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\PropertiesDialog.cpp

"$(INTDIR)\PropertiesDialog.obj"	"$(INTDIR)\PropertiesDialog.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RemotePropertiesDialog.cpp

"$(INTDIR)\RemotePropertiesDialog.obj"	"$(INTDIR)\RemotePropertiesDialog.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RunTestsDlg.cpp

"$(INTDIR)\RunTestsDlg.obj"	"$(INTDIR)\RunTestsDlg.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RunTestsSheet.cpp

"$(INTDIR)\RunTestsSheet.obj"	"$(INTDIR)\RunTestsSheet.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\StdAfx.cpp

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\SummaryPage.cpp

"$(INTDIR)\SummaryPage.obj"	"$(INTDIR)\SummaryPage.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\TestTool.cpp

!IF  "$(CFG)" == "TestTool - Win32 Release"

CPP_SWITCHES=/nologo /MD /W4 /GX /O2 /I "..\TestSheet" /I "." /I "..\..\ecostest\common" /I "..\common" /I "..\..\utils\common" /I "..\.." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\TestTool.obj"	"$(INTDIR)\TestTool.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "TestTool - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W4 /Gm /GX /ZI /Od /I "." /I "..\..\ecostest\common" /I "..\common" /I "..\..\utils\common" /I "C:\cvs\devo\ide\src\tools\eCosTest" /I "..\.." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\TestTool.obj"	"$(INTDIR)\TestTool.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\TestTool.rc

"$(INTDIR)\TestTool.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


!IF  "$(CFG)" == "TestTool - Win32 Release"

"eCosTest - Win32 Release" : 
   cd "\e\cvs\devo\ecos\host\tools\ecostest\win32"
   $(MAKE) /$(MAKEFLAGS) /F .\eCosTest.mak CFG="eCosTest - Win32 Release" 
   cd "..\..\testtool\win32"

"eCosTest - Win32 ReleaseCLEAN" : 
   cd "\e\cvs\devo\ecos\host\tools\ecostest\win32"
   $(MAKE) /$(MAKEFLAGS) /F .\eCosTest.mak CFG="eCosTest - Win32 Release" RECURSE=1 CLEAN 
   cd "..\..\testtool\win32"

!ELSEIF  "$(CFG)" == "TestTool - Win32 Debug"

"eCosTest - Win32 Debug" : 
   cd "\e\cvs\devo\ecos\host\tools\ecostest\win32"
   $(MAKE) /$(MAKEFLAGS) /F .\eCosTest.mak CFG="eCosTest - Win32 Debug" 
   cd "..\..\testtool\win32"

"eCosTest - Win32 DebugCLEAN" : 
   cd "\e\cvs\devo\ecos\host\tools\ecostest\win32"
   $(MAKE) /$(MAKEFLAGS) /F .\eCosTest.mak CFG="eCosTest - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\..\testtool\win32"

!ENDIF 


!ENDIF 

