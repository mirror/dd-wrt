# Microsoft Developer Studio Project File - Name="sf_engine" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=sf_engine - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sf_engine.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sf_engine.mak" CFG="sf_engine - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sf_engine - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "sf_engine - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sf_engine - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "SF_Engine_Release"
# PROP Intermediate_Dir "SF_Engine_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SF_ENGINE_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\dynamic-plugins" /I "..\..\dynamic-plugins\sf_engine" /I "..\Win32-Includes" /I "..\Win32-Includes\zlib" /I "..\..\..\daq\api" /I "..\..\..\daq\sfbpf" /D "NDEBUG" /D "ENABLE_PAF" /D "SF_SNORT_ENGINE_DLL" /D "_WINDOWS" /D "_USRDLL" /D "ACTIVE_RESPONSE" /D "GRE" /D "MPLS" /D "TARGET_BASED" /D "PERF_PROFILING" /D "ENABLE_RESPOND" /D "ENABLE_REACT" /D "ZLIB" /D "_WINDLL" /D "WIN32" /D "_MBCS" /D "HAVE_CONFIG_H" /D "_AFXDLL" /D SIGNAL_SNORT_READ_ATTR_TBL=30 /D BYTE_ORDER=LITTLE_ENDIAN /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 zlib1.lib Ws2_32.lib pcre.lib /nologo /dll /machine:I386 /libpath:"..\Win32-Libraries"

!ELSEIF  "$(CFG)" == "sf_engine - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "SF_Engine_Debug"
# PROP Intermediate_Dir "SF_Engine_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SF_ENGINE_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\Win32-Includes\zlib" /I "..\..\dynamic-plugins" /I "..\..\dynamic-plugins\sf_engine" /I "..\Win32-Includes" /I "..\..\..\daq\api" /I "..\..\..\daq\sfbpf" /D "_DEBUG" /D "DEBUG" /D "ENABLE_PAF" /D "SF_SNORT_ENGINE_DLL" /D "_WINDOWS" /D "_USRDLL" /D "ACTIVE_RESPONSE" /D "GRE" /D "MPLS" /D "TARGET_BASED" /D "PERF_PROFILING" /D "ENABLE_RESPOND" /D "ENABLE_REACT" /D "ZLIB" /D "_WINDLL" /D "WIN32" /D "_MBCS" /D "HAVE_CONFIG_H" /D "_AFXDLL" /D SIGNAL_SNORT_READ_ATTR_TBL=30 /D BYTE_ORDER=LITTLE_ENDIAN /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 zlib1.lib Ws2_32.lib pcre.lib /nologo /dll /debug /machine:I386 /pdbtype:sept /libpath:"..\Win32-Libraries"

!ENDIF 

# Begin Target

# Name "sf_engine - Win32 Release"
# Name "sf_engine - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\bmh.c"

!IF  "$(CFG)" == "sf_engine - Win32 Release"

# ADD CPP /D "MODULUS_HASH"

!ELSEIF  "$(CFG)" == "sf_engine - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\md5.c"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sf_decompression.c"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sf_sechash.c"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sf_snort_detection_engine.c"

!IF  "$(CFG)" == "sf_engine - Win32 Release"

# ADD CPP /I "..\..\.." /I "..\Win32-Includes\mysql" /I "..\Win32-Includes\libnet" /I "..\..\output-plugins" /I "..\..\detection-plugins" /I "..\..\preprocessors" /I "..\..\preprocessors\flow" /I "..\..\preprocessors\portscan" /I "..\..\preprocessors\flow\int-snort" /I "..\..\preprocessors\HttpInspect\Include" /D "MODULUS_HASH"

!ELSEIF  "$(CFG)" == "sf_engine - Win32 Debug"

# ADD CPP /I "..\..\.." /I "..\Win32-Includes\mysql" /I "..\Win32-Includes\libnet" /I "..\..\output-plugins" /I "..\..\detection-plugins" /I "..\..\preprocessors" /I "..\..\preprocessors\flow" /I "..\..\preprocessors\portscan" /I "..\..\preprocessors\flow\int-snort" /I "..\..\preprocessors\HttpInspect\Include"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sf_snort_plugin_api.c"

!IF  "$(CFG)" == "sf_engine - Win32 Release"

# ADD CPP /I "..\..\.." /I "..\Win32-Includes\mysql" /I "..\Win32-Includes\libnet" /I "..\..\output-plugins" /I "..\..\detection-plugins" /I "..\..\preprocessors" /I "..\..\preprocessors\flow" /I "..\..\preprocessors\portscan" /I "..\..\preprocessors\flow\int-snort" /I "..\..\preprocessors\HttpInspect\Include" /D "MODULUS_HASH"

!ELSEIF  "$(CFG)" == "sf_engine - Win32 Debug"

# ADD CPP /I "..\..\.." /I "..\Win32-Includes\mysql" /I "..\Win32-Includes\libnet" /I "..\..\output-plugins" /I "..\..\detection-plugins" /I "..\..\preprocessors" /I "..\..\preprocessors\flow" /I "..\..\preprocessors\portscan" /I "..\..\preprocessors\flow\int-snort" /I "..\..\preprocessors\HttpInspect\Include"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sf_snort_plugin_byte.c"

!IF  "$(CFG)" == "sf_engine - Win32 Release"

# ADD CPP /D "MODULUS_HASH"

!ELSEIF  "$(CFG)" == "sf_engine - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sf_snort_plugin_content.c"

!IF  "$(CFG)" == "sf_engine - Win32 Release"

# ADD CPP /D "MODULUS_HASH"

!ELSEIF  "$(CFG)" == "sf_engine - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sf_snort_plugin_hdropts.c"

!IF  "$(CFG)" == "sf_engine - Win32 Release"

# ADD CPP /D "MODULUS_HASH"

!ELSEIF  "$(CFG)" == "sf_engine - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sf_snort_plugin_loop.c"

!IF  "$(CFG)" == "sf_engine - Win32 Release"

# ADD CPP /D "MODULUS_HASH"

!ELSEIF  "$(CFG)" == "sf_engine - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sf_snort_plugin_pcre.c"

!IF  "$(CFG)" == "sf_engine - Win32 Release"

# ADD CPP /D "MODULUS_HASH"

!ELSEIF  "$(CFG)" == "sf_engine - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sf_snort_plugin_rc4.c"

!IF  "$(CFG)" == "sf_engine - Win32 Release"

# ADD CPP /D "MODULUS_HASH"

!ELSEIF  "$(CFG)" == "sf_engine - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sfghash.c"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sfhashfcn.c"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sfprimetable.c"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sha2.c"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\bmh.h"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\debug.h"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\ipv6_port.h"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\md5.h"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sf_decompression.h"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_decompression_define.h"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sf_ip.h"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sf_sechash.h"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sf_snort_detection_engine.h"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sf_snort_packet.h"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sf_snort_plugin_api.h"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sfghash.h"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sfhashfcn.h"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sfprimetable.h"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sha2.h"
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
