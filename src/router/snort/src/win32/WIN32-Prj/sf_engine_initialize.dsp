# Microsoft Developer Studio Project File - Name="sf_engine_initialize" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Generic Project" 0x010a

CFG=sf_engine_initialize - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sf_engine_initialize.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sf_engine_initialize.mak" CFG="sf_engine_initialize - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sf_engine_initialize - Win32 Release" (based on "Win32 (x86) Generic Project")
!MESSAGE "sf_engine_initialize - Win32 Debug" (based on "Win32 (x86) Generic Project")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
MTL=midl.exe

!IF  "$(CFG)" == "sf_engine_initialize - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "SF_Engine_Initialize_Release"
# PROP Intermediate_Dir "SF_Engine_Initialize_Release"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "sf_engine_initialize - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "SF_Engine_Initialize_Debug"
# PROP Intermediate_Dir "SF_Engine_Initialize_Debug"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "sf_engine_initialize - Win32 Release"
# Name "sf_engine_initialize - Win32 Debug"
# Begin Source File

SOURCE=..\..\ipv6_port.h

!IF  "$(CFG)" == "sf_engine_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\ipv6_port.h
InputName=ipv6_port

BuildCmds= \
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine\$(InputName).h.new \
	c:\cygwin\bin\sed -e "s/->iph->ip_src/->ip4_header->source/" -e "s/->iph->ip_dst/->ip4_header->destination/" -e "s/->iph->/->ip4_header->/" -e "s/->iph$/->ip4_header/" -e "s/orig_iph$/orig_ip4_header/" -e "s/ip_verhl/version_headerlength/" -e "s/ip_tos/type_service/" -e "s/ip_len/data_length/" -e "s/ip_id/identifier/" -e "s/ip_off/offset/" -e "s/ip_ttl/time_to_live/" -e "s/ip_proto/proto/" -e "s/ip_csum/checksum/" -e "s/p->iph$/p->ip4_header/" ../../dynamic-plugins/sf_engine/$(InputName).h.new > ../../dynamic-plugins/sf_engine/$(InputName).h \
	

"..\..\dynamic-plugins\sf_engine\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\..\dynamic-plugins\sf_engine\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_engine_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\ipv6_port.h
InputName=ipv6_port

BuildCmds= \
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine\$(InputName).h.new \
	c:\cygwin\bin\sed -e "s/->iph->ip_src/->ip4_header->source/" -e "s/->iph->ip_dst/->ip4_header->destination/" -e "s/->iph->/->ip4_header->/" -e "s/->iph$/->ip4_header/" -e "s/orig_iph$/orig_ip4_header/" -e "s/ip_verhl/version_headerlength/" -e "s/ip_tos/type_service/" -e "s/ip_len/data_length/" -e "s/ip_id/identifier/" -e "s/ip_off/offset/" -e "s/ip_ttl/time_to_live/" -e "s/ip_proto/proto/" -e "s/ip_csum/checksum/" -e "s/p->iph$/p->ip4_header/" ../../dynamic-plugins/sf_engine/$(InputName).h.new > ../../dynamic-plugins/sf_engine/$(InputName).h \
	

"..\..\dynamic-plugins\sf_engine\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\..\dynamic-plugins\sf_engine\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\md5.c

!IF  "$(CFG)" == "sf_engine_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\md5.c
InputName=md5

"..\..\dynamic-plugins\sf_engine\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ELSEIF  "$(CFG)" == "sf_engine_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\md5.c
InputName=md5

"..\..\dynamic-plugins\sf_engine\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\md5.h

!IF  "$(CFG)" == "sf_engine_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\md5.h
InputName=md5

"..\..\dynamic-plugins\sf_engine\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ELSEIF  "$(CFG)" == "sf_engine_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\md5.h
InputName=md5

"..\..\dynamic-plugins\sf_engine\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\preprocids.h

!IF  "$(CFG)" == "sf_engine_initialize - Win32 Release"

!ELSEIF  "$(CFG)" == "sf_engine_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\preprocids.h
InputName=preprocids

"..\..\dynamic-plugins\sf_engine\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
       copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ENDIF

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_ip.c

!IF  "$(CFG)" == "sf_engine_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sf_ip.c
InputName=sf_ip

"..\..\dynamic-plugins\sf_engine\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ELSEIF  "$(CFG)" == "sf_engine_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sf_ip.c
InputName=sf_ip

"..\..\dynamic-plugins\sf_engine\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_ip.h

!IF  "$(CFG)" == "sf_engine_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sf_ip.h
InputName=sf_ip

"..\..\dynamic-plugins\sf_engine\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ELSEIF  "$(CFG)" == "sf_engine_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sf_ip.h
InputName=sf_ip

"..\..\dynamic-plugins\sf_engine\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sf_protocols.h

!IF  "$(CFG)" == "sf_engine_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sf_protocols.h
InputName=sf_protocols

"..\..\dynamic-plugins\sf_engine\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ELSEIF  "$(CFG)" == "sf_engine_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sf_protocols.h
InputName=sf_protocols

"..\..\dynamic-plugins\sf_engine\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_sechash.c

!IF  "$(CFG)" == "sf_engine_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sf_sechash.c
InputName=sf_sechash

"..\..\dynamic-plugins\sf_engine\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ELSEIF  "$(CFG)" == "sf_engine_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sf_sechash.c
InputName=sf_sechash

"..\..\dynamic-plugins\sf_engine\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_sechash.h

!IF  "$(CFG)" == "sf_engine_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sf_sechash.h
InputName=sf_sechash

"..\..\dynamic-plugins\sf_engine\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ELSEIF  "$(CFG)" == "sf_engine_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sf_sechash.h
InputName=sf_sechash

"..\..\dynamic-plugins\sf_engine\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sf_types.h

!IF  "$(CFG)" == "sf_engine_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sf_types.h
InputName=sf_types

"..\..\dynamic-plugins\sf_engine\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ELSEIF  "$(CFG)" == "sf_engine_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sf_types.h
InputName=sf_types

"..\..\dynamic-plugins\sf_engine\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfghash.c

!IF  "$(CFG)" == "sf_engine_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sfghash.c
InputName=sfghash

"..\..\dynamic-plugins\sf_engine\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ELSEIF  "$(CFG)" == "sf_engine_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sfghash.c
InputName=sfghash

"..\..\dynamic-plugins\sf_engine\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfghash.h

!IF  "$(CFG)" == "sf_engine_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sfghash.h
InputName=sfghash

"..\..\dynamic-plugins\sf_engine\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ELSEIF  "$(CFG)" == "sf_engine_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sfghash.h
InputName=sfghash

"..\..\dynamic-plugins\sf_engine\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfhashfcn.c

!IF  "$(CFG)" == "sf_engine_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sfhashfcn.c
InputName=sfhashfcn

BuildCmds= \
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine\$(InputName).c.new \
	c:\cygwin\bin\sed -e "s/\#ifndef MODULUS_HASH/\#ifdef STATIC_HASH/" ../../dynamic-plugins/sf_engine/$(InputName).c.new > ../../dynamic-plugins/sf_engine/$(InputName).c \
	

"..\..\dynamic-plugins\sf_engine\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\..\dynamic-plugins\sf_engine\$(InputName).c.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_engine_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sfhashfcn.c
InputName=sfhashfcn

BuildCmds= \
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine\$(InputName).c.new \
	c:\cygwin\bin\sed -e "s/\#ifndef MODULUS_HASH/\#ifdef STATIC_HASH/" ../../dynamic-plugins/sf_engine/$(InputName).c.new > ../../dynamic-plugins/sf_engine/$(InputName).c \
	

"..\..\dynamic-plugins\sf_engine\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\..\dynamic-plugins\sf_engine\$(InputName).c.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfhashfcn.h

!IF  "$(CFG)" == "sf_engine_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sfhashfcn.h
InputName=sfhashfcn

"..\..\dynamic-plugins\sf_engine\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ELSEIF  "$(CFG)" == "sf_engine_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sfhashfcn.h
InputName=sfhashfcn

"..\..\dynamic-plugins\sf_engine\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfprimetable.c

!IF  "$(CFG)" == "sf_engine_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sfprimetable.c
InputName=sfprimetable

"..\..\dynamic-plugins\sf_engine\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ELSEIF  "$(CFG)" == "sf_engine_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sfprimetable.c
InputName=sfprimetable

"..\..\dynamic-plugins\sf_engine\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfprimetable.h

!IF  "$(CFG)" == "sf_engine_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sfprimetable.h
InputName=sfprimetable

"..\..\dynamic-plugins\sf_engine\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ELSEIF  "$(CFG)" == "sf_engine_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sfprimetable.h
InputName=sfprimetable

"..\..\dynamic-plugins\sf_engine\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sha2.c

!IF  "$(CFG)" == "sf_engine_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sha2.c
InputName=sha2

"..\..\dynamic-plugins\sf_engine\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ELSEIF  "$(CFG)" == "sf_engine_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sha2.c
InputName=sha2

"..\..\dynamic-plugins\sf_engine\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sha2.h

!IF  "$(CFG)" == "sf_engine_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sha2.h
InputName=sha2

"..\..\dynamic-plugins\sf_engine\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ELSEIF  "$(CFG)" == "sf_engine_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sha2.h
InputName=sha2

"..\..\dynamic-plugins\sf_engine\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\snort_debug.h

!IF  "$(CFG)" == "sf_engine_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\snort_debug.h
InputName=snort_debug

BuildCmds= \
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine\$(InputName).h.new \
	c:\cygwin\bin\sed -e "s/DebugMessageFile = /*_ded.debugMsgFile = /" -e "s/DebugMessageLine = /*_ded.debugMsgLine = /" -e "s/; DebugMessageFunc$/; _ded.debugMsg/" -e "s/; DebugWideMessageFunc$/; _ded.debugWideMsg/" ../../dynamic-plugins\sf_engine\$(InputName).h.new > ../../dynamic-plugins/sf_engine/$(InputName).h \
	

"..\..\dynamic-plugins\sf_engine\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\..\dynamic-plugins\sf_engine\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_engine_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\snort_debug.h
InputName=snort_debug

BuildCmds= \
	copy $(InputPath) ..\..\dynamic-plugins\sf_engine\$(InputName).h.new \
	c:\cygwin\bin\sed -e "s/DebugMessageFile = /*_ded.debugMsgFile = /" -e "s/DebugMessageLine = /*_ded.debugMsgLine = /" -e "s/; DebugMessageFunc$/; _ded.debugMsg/" -e "s/; DebugWideMessageFunc$/; _ded.debugWideMsg/" ../../dynamic-plugins/sf_engine/$(InputName).h.new > ../../dynamic-plugins/sf_engine/$(InputName).h \
	

"..\..\dynamic-plugins\sf_engine\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\..\dynamic-plugins\sf_engine\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Target
# End Project
