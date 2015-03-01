# Microsoft Developer Studio Project File - Name="sf_dynamic_initialize" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Generic Project" 0x010a

CFG=sf_dynamic_initialize - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sf_dynamic_initialize.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sf_dynamic_initialize.mak" CFG="sf_dynamic_initialize - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sf_dynamic_initialize - Win32 Release" (based on "Win32 (x86) Generic Project")
!MESSAGE "sf_dynamic_initialize - Win32 Debug" (based on "Win32 (x86) Generic Project")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
MTL=midl.exe

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "sf_dynamic_initialize - Win32 Release"
# Name "sf_dynamic_initialize - Win32 Debug"
# Begin Source File

SOURCE=..\..\sfutil\bitop.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\bitop.h
InputName=bitop

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\bitop.h
InputName=bitop

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\cpuclock.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\cpuclock.h
InputName=cpuclock

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\cpuclock.h
InputName=cpuclock

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\event.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\event.h
InputName=event

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\event.h
InputName=event

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\file-process\file_api.h"

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath="..\..\file-process\file_api.h"
InputName=file_api

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath="..\..\file-process\file_api.h"
InputName=file_api

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\file-process\file_mail_common.h"

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath="..\..\file-process\file_mail_common.h"
InputName=file_mail_common

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath="..\..\file-process\file_mail_common.h"
InputName=file_mail_common

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\idle_processing.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\idle_processing.h
InputName=idle_processing

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\idle_processing.h
InputName=idle_processing

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\win32\WIN32-Code\inet_aton.c"

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath="..\..\win32\WIN32-Code\inet_aton.c"
InputName=inet_aton

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath="..\..\win32\WIN32-Code\inet_aton.c"
InputName=inet_aton

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\win32\WIN32-Code\inet_pton.c"

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath="..\..\win32\WIN32-Code\inet_pton.c"
InputName=inet_pton

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath="..\..\win32\WIN32-Code\inet_pton.c"
InputName=inet_pton

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ipv6_port.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\ipv6_port.h
InputName=ipv6_port

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -e "s/->iph->ip_src/->ip4_header->source/" -e "s/->iph->ip_dst/->ip4_header->destination/" -e "s/->iph->/->ip4_header->/" -e "s/->iph$/->ip4_header/" -e "s/orig_iph$/orig_ip4_header/" -e "s/ip_verhl/version_headerlength/" -e "s/ip_tos/type_service/" -e "s/ip_len/data_length/" -e "s/ip_id/identifier/" -e "s/ip_off/offset/" -e "s/ip_ttl/time_to_live/" -e "s/ip_proto/proto/" -e "s/ip_csum/checksum/" -e "s/p->iph$/p->ip4_header/" ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\ipv6_port.h
InputName=ipv6_port

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -e "s/->iph->ip_src/->ip4_header->source/" -e "s/->iph->ip_dst/->ip4_header->destination/" -e "s/->iph->/->ip4_header->/" -e "s/->iph$/->ip4_header/" -e "s/orig_iph$/orig_ip4_header/" -e "s/ip_verhl/version_headerlength/" -e "s/ip_tos/type_service/" -e "s/ip_len/data_length/" -e "s/ip_id/identifier/" -e "s/ip_off/offset/" -e "s/ip_ttl/time_to_live/" -e "s/ip_proto/proto/" -e "s/ip_csum/checksum/" -e "s/p->iph$/p->ip4_header/" ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\mempool.c

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\mempool.c
InputName=mempool

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).c.new \
	c:\cygwin\bin\sed -e "/SharedObjectAddStarts/d" -e "/SharedObjectAddEnds/d" -e "/SharedObjectDeleteBegins/,/SharedObjectDeleteEnds/d" -e "s/getDefaultPolicy()/_dpd.getDefaultPolicy()/" -e "s/ErrorMessage/_dpd.errMsg/" -e "s/LogMessage /_dpd.logMsg /" -e "/util.h/d" ../include/$(InputName).c.new > ../include/$(InputName).c \
	

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).c.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\mempool.c
InputName=mempool

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).c.new \
	c:\cygwin\bin\sed -e "/SharedObjectAddStarts/d" -e "/SharedObjectAddEnds/d" -e "/SharedObjectDeleteBegins/,/SharedObjectDeleteEnds/d" -e "s/getDefaultPolicy()/_dpd.getDefaultPolicy()/" -e "s/ErrorMessage/_dpd.errMsg/" -e "s/LogMessage /_dpd.logMsg /" -e "/util.h/d" ../include/$(InputName).c.new > ../include/$(InputName).c \
	

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).c.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\mempool.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\mempool.h
InputName=mempool

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -e "/SharedObjectAddStarts/d" -e "/SharedObjectAddEnds/d" -e "/SharedObjectDeleteBegins/,/SharedObjectDeleteEnds/d" -e "s/getDefaultPolicy()/_dpd.getDefaultPolicy()/" -e "s/ErrorMessage/_dpd.errMsg/" -e "s/LogMessage /_dpd.logMsg /" -e "/util.h/d" ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\mempool.h
InputName=mempool

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -e "/SharedObjectAddStarts/d" -e "/SharedObjectAddEnds/d" -e "/SharedObjectDeleteBegins/,/SharedObjectDeleteEnds/d" -e "s/getDefaultPolicy()/_dpd.getDefaultPolicy()/" -e "s/ErrorMessage/_dpd.errMsg/" -e "s/LogMessage /_dpd.logMsg /" -e "/util.h/d" ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\mpse_methods.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\mpse_methods.h
InputName=mpse_methods

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include\$(InputName).h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\mpse_methods.h
InputName=mpse_methods

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include\$(InputName).h 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\obfuscation.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\obfuscation.h
InputName=obfuscation

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -e "s/Packet /SFSnortPacket /" -e "s/decode.h/sf_snort_packet.h/" -e "/sfportobject\.h/d" -e "s/PortObject/void/g" ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\obfuscation.h
InputName=obfuscation

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -e "s/Packet /SFSnortPacket /" -e "s/decode.h/sf_snort_packet.h/" -e "/sfportobject\.h/d" -e "s/PortObject/void/g" ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\pcap_pkthdr32.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\pcap_pkthdr32.h
InputName=pcap_pkthdr32

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\pcap_pkthdr32.h
InputName=pcap_pkthdr32

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\plugin_enum.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\plugin_enum.h
InputName=plugin_enum

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\plugin_enum.h
InputName=plugin_enum

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\preprocids.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\preprocids.h
InputName=preprocids

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\preprocids.h
InputName=preprocids

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\profiler.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\profiler.h
InputName=profiler

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\profiler.h
InputName=profiler

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\rule_option_types.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\rule_option_types.h
InputName=rule_option_types

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\rule_option_types.h
InputName=rule_option_types

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\segment_mem.c

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\segment_mem.c
InputName=segment_mem

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\segment_mem.c
InputName=segment_mem

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\segment_mem.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\segment_mem.h
InputName=segment_mem

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\segment_mem.h
InputName=segment_mem

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\session_api.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\preprocessors\session_api.h
InputName=session_api

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -e "s/Packet /SFSnortPacket /" -e "s/decode.h/sf_snort_packet.h/" -e "/sfportobject\.h/d" -e "s/PortObject/void/g" ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\preprocessors\session_api.h
InputName=session_api

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -e "s/Packet /SFSnortPacket /" -e "s/decode.h/sf_snort_packet.h/" -e "/sfportobject\.h/d" -e "s/PortObject/void/g" ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\Session\session_common.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\preprocessors\Session\session_common.h
InputName=session_common

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\preprocessors\Session\session_common.h
InputName=session_common

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\Session\session_expect.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\preprocessors\Session\session_expect.h
InputName=session_expect

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\preprocessors\Session\session_expect.h
InputName=session_expect

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_base64decode.c

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sf_base64decode.c
InputName=sf_base64decode

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sf_base64decode.c
InputName=sf_base64decode

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_base64decode.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sf_base64decode.h
InputName=sf_base64decode

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sf_base64decode.h
InputName=sf_base64decode

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_dynamic_common.h"

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_dynamic_common.h"
InputName=sf_dynamic_common

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_dynamic_common.h"
InputName=sf_dynamic_common

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_dynamic_define.h"

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_dynamic_define.h"
InputName=sf_dynamic_define

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_dynamic_define.h"
InputName=sf_dynamic_define

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_dynamic_engine.h"

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_dynamic_engine.h"
InputName=sf_dynamic_engine

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_dynamic_engine.h"
InputName=sf_dynamic_engine

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_dynamic_meta.h"

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_dynamic_meta.h"
InputName=sf_dynamic_meta

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_dynamic_meta.h"
InputName=sf_dynamic_meta

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_preproc_example\sf_dynamic_preproc_lib.c"

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_preproc_example\sf_dynamic_preproc_lib.c"
InputName=sf_dynamic_preproc_lib

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_preproc_example\sf_dynamic_preproc_lib.c"
InputName=sf_dynamic_preproc_lib

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_preproc_example\sf_dynamic_preproc_lib.h"

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_preproc_example\sf_dynamic_preproc_lib.h"
InputName=sf_dynamic_preproc_lib

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_preproc_example\sf_dynamic_preproc_lib.h"
InputName=sf_dynamic_preproc_lib

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_dynamic_preprocessor.h"

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_dynamic_preprocessor.h"
InputName=sf_dynamic_preprocessor

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -e "s/Packet /SFSnortPacket /" -e "s/decode.h/sf_snort_packet.h/" -e "/sfportobject\.h/d" -e "s/PortObject/void/g" ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_dynamic_preprocessor.h"
InputName=sf_dynamic_preprocessor

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -e "s/Packet /SFSnortPacket /" -e "s/decode.h/sf_snort_packet.h/" -e "/sfportobject\.h/d" -e "s/PortObject/void/g" ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_email_attach_decode.c

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sf_email_attach_decode.c
InputName=sf_email_attach_decode

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).c.new \
	c:\cygwin\bin\sed -e "/SharedObjectAddStarts/d" -e "/SharedObjectAddEnds/d" -e "/SharedObjectDeleteBegins/,/SharedObjectDeleteEnds/d" -e "s/SnortStrnStr/_dpd.SnortStrnStr/" -e "/util.h/d" ../include/$(InputName).c.new > ../include/$(InputName).c \
	

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).c.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sf_email_attach_decode.c
InputName=sf_email_attach_decode

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).c.new \
	c:\cygwin\bin\sed -e "/SharedObjectAddStarts/d" -e "/SharedObjectAddEnds/d" -e "/SharedObjectDeleteBegins/,/SharedObjectDeleteEnds/d" -e "s/SnortStrnStr/_dpd.SnortStrnStr/" -e "/util.h/d" ../include/$(InputName).c.new > ../include/$(InputName).c \
	

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).c.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_email_attach_decode.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sf_email_attach_decode.h
InputName=sf_email_attach_decode

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sf_email_attach_decode.h
InputName=sf_email_attach_decode

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_ip.c

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sf_ip.c
InputName=sf_ip

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sf_ip.c
InputName=sf_ip

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_ip.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sf_ip.h
InputName=sf_ip

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sf_ip.h
InputName=sf_ip

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sf_protocols.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sf_protocols.h
InputName=sf_protocols

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sf_protocols.h
InputName=sf_protocols

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sf_sdlist.c

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sf_sdlist.c
InputName=sf_sdlist

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).c.new \
	c:\cygwin\bin\sed -e "/SharedObjectAddStarts/d" -e "/SharedObjectAddEnds/d" -e "/SharedObjectDeleteBegins/,/SharedObjectDeleteEnds/d" -e "s/getDefaultPolicy()/_dpd.getDefaultPolicy()/" -e "s/ErrorMessage/_dpd.errMsg/" -e "s/LogMessage /_dpd.logMsg /" -e "/util.h/d" ../include/$(InputName).c.new > ../include/$(InputName).c \
	

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).c.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sf_sdlist.c
InputName=sf_sdlist

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).c.new \
	c:\cygwin\bin\sed -e "/SharedObjectAddStarts/d" -e "/SharedObjectAddEnds/d" -e "/SharedObjectDeleteBegins/,/SharedObjectDeleteEnds/d" -e "s/getDefaultPolicy()/_dpd.getDefaultPolicy()/" -e "s/ErrorMessage/_dpd.errMsg/" -e "s/LogMessage /_dpd.logMsg /" -e "/util.h/d" ../include/$(InputName).c.new > ../include/$(InputName).c \
	

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).c.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sf_sdlist.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sf_sdlist.h
InputName=sf_sdlist

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -e "/SharedObjectAddStarts/d" -e "/SharedObjectAddEnds/d" -e "/SharedObjectDeleteBegins/,/SharedObjectDeleteEnds/d" -e "s/getDefaultPolicy()/_dpd.getDefaultPolicy()/" -e "s/ErrorMessage/_dpd.errMsg/" -e "s/LogMessage /_dpd.logMsg /" -e "/util.h/d" ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sf_sdlist.h
InputName=sf_sdlist

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -e "/SharedObjectAddStarts/d" -e "/SharedObjectAddEnds/d" -e "/SharedObjectDeleteBegins/,/SharedObjectDeleteEnds/d" -e "s/getDefaultPolicy()/_dpd.getDefaultPolicy()/" -e "s/ErrorMessage/_dpd.errMsg/" -e "s/LogMessage /_dpd.logMsg /" -e "/util.h/d" ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sf_sdlist_types.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sf_sdlist_types.h
InputName=sf_sdlist_types

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -e "/SharedObjectAddStarts/d" -e "/SharedObjectAddEnds/d" -e "/SharedObjectDeleteBegins/,/SharedObjectDeleteEnds/d" -e "s/getDefaultPolicy()/_dpd.getDefaultPolicy()/" -e "s/ErrorMessage/_dpd.errMsg/" -e "s/LogMessage /_dpd.logMsg /" -e "/util.h/d" ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sf_sdlist_types.h
InputName=sf_sdlist_types

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -e "/SharedObjectAddStarts/d" -e "/SharedObjectAddEnds/d" -e "/SharedObjectDeleteBegins/,/SharedObjectDeleteEnds/d" -e "s/getDefaultPolicy()/_dpd.getDefaultPolicy()/" -e "s/ErrorMessage/_dpd.errMsg/" -e "s/LogMessage /_dpd.logMsg /" -e "/util.h/d" ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_seqnums.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sf_seqnums.h
InputName=sf_seqnums

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sf_seqnums.h
InputName=sf_seqnums

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sf_snort_packet.h"

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_engine\sf_snort_packet.h"
InputName=sf_snort_packet

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_engine\sf_snort_packet.h"
InputName=sf_snort_packet

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sf_snort_plugin_api.h"

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_engine\sf_snort_plugin_api.h"
InputName=sf_snort_plugin_api

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_engine\sf_snort_plugin_api.h"
InputName=sf_snort_plugin_api

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sf_types.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sf_types.h
InputName=sf_types

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sf_types.h
InputName=sf_types

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\control\sfcontrol.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\control\sfcontrol.h
InputName=sfcontrol

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\control\sfcontrol.h
InputName=sfcontrol

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfhashfcn.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sfhashfcn.h
InputName=sfhashfcn

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sfhashfcn.h
InputName=sfhashfcn

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\libs\sfparser.c
# End Source File
# Begin Source File

SOURCE="..\..\sfutil\sfPolicy.h"

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath="..\..\sfutil\sfPolicy.h"
InputName=sfPolicy

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -e "/SharedObjectAddStarts/d" -e "/SharedObjectAddEnds/d" -e "/SharedObjectDeleteBegins/,/SharedObjectDeleteEnds/d" -e "s/getDefaultPolicy()/_dpd.getDefaultPolicy()/" ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath="..\..\sfutil\sfPolicy.h"
InputName=sfPolicy

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -e "/SharedObjectAddStarts/d" -e "/SharedObjectAddEnds/d" -e "/SharedObjectDeleteBegins/,/SharedObjectDeleteEnds/d" -e "s/getDefaultPolicy()/_dpd.getDefaultPolicy()/" ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\sfutil\sfPolicyUserData.c"

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath="..\..\sfutil\sfPolicyUserData.c"
InputName=sfPolicyUserData

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).c.new \
	c:\cygwin\bin\sed -e "/SharedObjectAddStarts/d" -e "/SharedObjectAddEnds/d" -e "/SharedObjectDeleteBegins/,/SharedObjectDeleteEnds/d" -e "s/getDefaultPolicy()/_dpd.getDefaultPolicy()/" ../include/$(InputName).c.new > ../include/$(InputName).c \
	

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).c.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath="..\..\sfutil\sfPolicyUserData.c"
InputName=sfPolicyUserData

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).c.new \
	c:\cygwin\bin\sed -e "/SharedObjectAddStarts/d" -e "/SharedObjectAddEnds/d" -e "/SharedObjectDeleteBegins/,/SharedObjectDeleteEnds/d" -e "s/getDefaultPolicy()/_dpd.getDefaultPolicy()/" ../include/$(InputName).c.new > ../include/$(InputName).c \
	

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).c.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\sfutil\sfPolicyUserData.h"

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath="..\..\sfutil\sfPolicyUserData.h"
InputName=sfPolicyUserData

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -e "/SharedObjectAddStarts/d" -e "/SharedObjectAddEnds/d" -e "/SharedObjectDeleteBegins/,/SharedObjectDeleteEnds/d" -e "s/getDefaultPolicy()/_dpd.getDefaultPolicy()/" ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath="..\..\sfutil\sfPolicyUserData.h"
InputName=sfPolicyUserData

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -e "/SharedObjectAddStarts/d" -e "/SharedObjectAddEnds/d" -e "/SharedObjectDeleteBegins/,/SharedObjectDeleteEnds/d" -e "s/getDefaultPolicy()/_dpd.getDefaultPolicy()/" ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfrt.c

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sfrt.c
InputName=sfrt

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sfrt.c
InputName=sfrt

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfrt.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sfrt.h
InputName=sfrt

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sfrt.h
InputName=sfrt

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfrt_dir.c

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sfrt_dir.c
InputName=sfrt_dir

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sfrt_dir.c
InputName=sfrt_dir

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfrt_dir.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sfrt_dir.h
InputName=sfrt_dir

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sfrt_dir.h
InputName=sfrt_dir

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfrt_flat.c

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sfrt_flat.c
InputName=sfrt_flat

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sfrt_flat.c
InputName=sfrt_flat

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfrt_flat.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sfrt_flat.h
InputName=sfrt_flat

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sfrt_flat.h
InputName=sfrt_flat

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfrt_flat_dir.c

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sfrt_flat_dir.c
InputName=sfrt_flat_dir

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sfrt_flat_dir.c
InputName=sfrt_flat_dir

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfrt_flat_dir.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sfrt_flat_dir.h
InputName=sfrt_flat_dir

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sfrt_flat_dir.h
InputName=sfrt_flat_dir

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfrt_trie.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sfrt_trie.h
InputName=sfrt_trie

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sfrt_trie.h
InputName=sfrt_trie

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\examples\sfsnort_dynamic_detection_lib.c"

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_engine\examples\sfsnort_dynamic_detection_lib.c"
InputName=sfsnort_dynamic_detection_lib

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_engine\examples\sfsnort_dynamic_detection_lib.c"
InputName=sfsnort_dynamic_detection_lib

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\examples\sfsnort_dynamic_detection_lib.h"

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_engine\examples\sfsnort_dynamic_detection_lib.h"
InputName=sfsnort_dynamic_detection_lib

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_engine\examples\sfsnort_dynamic_detection_lib.h"
InputName=sfsnort_dynamic_detection_lib

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfxhash.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\sfxhash.h
InputName=sfxhash

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\sfxhash.h
InputName=sfxhash

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\signature.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\signature.h
InputName=signature

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -f ..\treenodes.sed ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\signature.h
InputName=signature

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -f ..\treenodes.sed ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\sip_common.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\preprocessors\sip_common.h
InputName=sip_common

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\preprocessors\sip_common.h
InputName=sip_common

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\snort_bounds.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\snort_bounds.h
InputName=snort_bounds

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\snort_bounds.h
InputName=snort_bounds

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\snort_debug.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\snort_debug.h
InputName=snort_debug

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -e "s/DebugMessageFile = /*_dpd.debugMsgFile = /" -e "s/DebugMessageLine = /*_dpd.debugMsgLine = /" -e "s/; DebugMessageFunc$/; _dpd.debugMsg/" -e "s/; DebugWideMessageFunc$/; _dpd.debugWideMsg/" ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\snort_debug.h
InputName=snort_debug

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -e "s/DebugMessageFile = /*_dpd.debugMsgFile = /" -e "s/DebugMessageLine = /*_dpd.debugMsgLine = /" -e "s/; DebugMessageFunc$/; _dpd.debugMsg/" -e "s/; DebugWideMessageFunc$/; _dpd.debugWideMsg/" ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\Session\snort_session.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\preprocessors\Session\snort_session.h
InputName=snort_session

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\preprocessors\Session\snort_session.h
InputName=snort_session

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\ssl_common\ssl.c
# End Source File
# Begin Source File

SOURCE=..\ssl_common\ssl.h
# End Source File
# Begin Source File

SOURCE=..\ssl_common\ssl_config.c
# End Source File
# Begin Source File

SOURCE=..\ssl_common\ssl_config.h
# End Source File
# Begin Source File

SOURCE=..\ssl_common\ssl_ha.c
# End Source File
# Begin Source File

SOURCE=..\ssl_common\ssl_ha.h
# End Source File
# Begin Source File

SOURCE=..\ssl_common\ssl_include.h
# End Source File
# Begin Source File

SOURCE=..\ssl_common\ssl_inspect.c
# End Source File
# Begin Source File

SOURCE=..\ssl_common\ssl_inspect.h
# End Source File
# Begin Source File

SOURCE=..\ssl_common\ssl_session.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\str_search.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\preprocessors\str_search.h
InputName=str_search

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -e "s/Packet /SFSnortPacket /" -e "s/decode.h/sf_snort_packet.h/" -e "/sfportobject\.h/d" -e "s/PortObject/void/g" ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\preprocessors\str_search.h
InputName=str_search

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -e "s/Packet /SFSnortPacket /" -e "s/decode.h/sf_snort_packet.h/" -e "/sfportobject\.h/d" -e "s/PortObject/void/g" ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\stream_api.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\preprocessors\stream_api.h
InputName=stream_api

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -e "s/Packet /SFSnortPacket /" -e "s/decode.h/sf_snort_packet.h/" -e "/sfportobject\.h/d" -e "s/PortObject/void/g" ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\preprocessors\stream_api.h
InputName=stream_api

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -e "s/Packet /SFSnortPacket /" -e "s/decode.h/sf_snort_packet.h/" -e "/sfportobject\.h/d" -e "s/PortObject/void/g" ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\Stream6\stream_common.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\preprocessors\Stream6\stream_common.h
InputName=stream_common

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\preprocessors\Stream6\stream_common.h
InputName=stream_common

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\win32\WIN32-Code\strtok_r.c"

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath="..\..\win32\WIN32-Code\strtok_r.c"
InputName=strtok_r

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath="..\..\win32\WIN32-Code\strtok_r.c"
InputName=strtok_r

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\treenodes.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\treenodes.h
InputName=treenodes

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -f ..\treenodes.sed ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\treenodes.h
InputName=treenodes

BuildCmds= \
	mkdir ..\include \
	copy $(InputPath) ..\include\$(InputName).h.new \
	c:\cygwin\bin\sed -f ..\treenodes.sed ../include/$(InputName).h.new > ../include/$(InputName).h \
	

"..\include\$(InputName).h.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\Unified2_common.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\Unified2_common.h
InputName=Unified2_common

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\Unified2_common.h
InputName=Unified2_common

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\util_unfold.c

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\util_unfold.c
InputName=util_unfold

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\util_unfold.c
InputName=util_unfold

"..\include\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\sfutil\util_unfold.h

!IF  "$(CFG)" == "sf_dynamic_initialize - Win32 Release"

# Begin Custom Build
InputPath=..\..\sfutil\util_unfold.h
InputName=util_unfold

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "sf_dynamic_initialize - Win32 Debug"

# Begin Custom Build
InputPath=..\..\sfutil\util_unfold.h
InputName=util_unfold

"..\include\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mkdir ..\include 
	copy $(InputPath) ..\include 
	
# End Custom Build

!ENDIF 

# End Source File
# End Target
# End Project
