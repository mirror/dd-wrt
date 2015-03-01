# Microsoft Developer Studio Project File - Name="snort_initialize" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Generic Project" 0x010a

CFG=snort_initialize - Win32 IPv6 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "snort_initialize.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "snort_initialize.mak" CFG="snort_initialize - Win32 IPv6 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "snort_initialize - Win32 Release" (based on "Win32 (x86) Generic Project")
!MESSAGE "snort_initialize - Win32 Debug" (based on "Win32 (x86) Generic Project")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
MTL=midl.exe

!IF  "$(CFG)" == "snort_initialize - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Snort_Initialize_Release"
# PROP Intermediate_Dir "Snort_Initialize_Release"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "snort_initialize - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Snort_Initialize_Debug"
# PROP Intermediate_Dir "Snort_Initialize_Debug"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "snort_initialize - Win32 Release"
# Name "snort_initialize - Win32 Debug"
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sf_snort_packet.h"

!IF  "$(CFG)" == "snort_initialize - Win32 Release"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_engine\sf_snort_packet.h"
InputName=sf_snort_packet

"..\..\detection-plugins\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\detection-plugins

# End Custom Build

!ELSEIF  "$(CFG)" == "snort_initialize - Win32 Debug"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_engine\sf_snort_packet.h"
InputName=sf_snort_packet

"..\..\detection-plugins\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\detection-plugins

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sf_snort_plugin_api.h"

!IF  "$(CFG)" == "snort_initialize - Win32 Release"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_engine\sf_snort_plugin_api.h"
InputName=sf_snort_plugin_api

"..\..\detection-plugins\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\detection-plugins

# End Custom Build

!ELSEIF  "$(CFG)" == "snort_initialize - Win32 Debug"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_engine\sf_snort_plugin_api.h"
InputName=sf_snort_plugin_api

"..\..\detection-plugins\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\detection-plugins

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_engine\sf_snort_plugin_hdropts.c"

!IF  "$(CFG)" == "snort_initialize - Win32 Release"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_engine\sf_snort_plugin_hdropts.c"
InputName=sf_snort_plugin_hdropts

BuildCmds= \
	copy $(InputPath) ..\..\detection-plugins\$(InputName).c.new \
	c:\cygwin\bin\sed -e "s/_ded.errMsg/ErrorMessage/g" -e "s/sf_snort_packet.h/decode.h/g" -e "s/SFSnortPacket/Packet/g" -e "s/ip4_header/iph/g" -e "s/tcp_header/tcph/g" -e "s/proto/ip_proto/g" -e "s/type_service/ip_tos/g" -e "s/time_to_live/ip_ttl/g" -e "s/num_ip_options/ip_option_count/g" -e "s/IPOptions/Options/g" -e "s/option_code/code/g" -e  "s/acknowledgement/th_ack/g" -e "s/sequence/th_seq/g" -e "s/tcph->flags/tcph->th_flags/g" -e "s/tcph->window/tcph->th_win/g" -e "s/num_tcp_options/tcp_option_count/g" -e "s/icmp_header/icmph/g" -e "s/ICMP_ECHO_REPLY/ICMP_ECHOREPLY/g" -e "s/ICMP_ECHO_REQUEST/ICMP_ECHO/g" -e "s/icmph_union.echo.id/s_icmp_id/g" -e "s/icmph_union.echo.seq/s_icmp_seq/g" -e "/sf_snort_detection_engine.h/d" ../../detection-plugins/$(InputName).c.new > ../../detection-plugins/$(InputName).c \
	

"..\..\detection-plugins\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\..\detection-plugins\$(InputName).c.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "snort_initialize - Win32 Debug"

# Begin Custom Build
InputPath="..\..\dynamic-plugins\sf_engine\sf_snort_plugin_hdropts.c"
InputName=sf_snort_plugin_hdropts

BuildCmds= \
	copy $(InputPath) ..\..\detection-plugins\$(InputName).c.new \
	c:\cygwin\bin\sed -e "s/_ded.errMsg/ErrorMessage/g" -e "s/sf_snort_packet.h/decode.h/g" -e "s/SFSnortPacket/Packet/g" -e "s/ip4_header/iph/g" -e "s/tcp_header/tcph/g" -e "s/proto/ip_proto/g" -e "s/type_service/ip_tos/g" -e "s/time_to_live/ip_ttl/g" -e "s/num_ip_options/ip_option_count/g" -e "s/IPOptions/Options/g" -e "s/option_code/code/g" -e  "s/acknowledgement/th_ack/g" -e "s/sequence/th_seq/g" -e "s/tcph->flags/tcph->th_flags/g" -e "s/tcph->window/tcph->th_win/g" -e "s/num_tcp_options/tcp_option_count/g" -e "s/icmp_header/icmph/g" -e "s/ICMP_ECHO_REPLY/ICMP_ECHOREPLY/g" -e "s/ICMP_ECHO_REQUEST/ICMP_ECHO/g" -e "s/icmph_union.echo.id/s_icmp_id/g" -e "s/icmph_union.echo.seq/s_icmp_seq/g" ../../detection-plugins/$(InputName).c.new > ../../detection-plugins/$(InputName).c \
	

"..\..\detection-plugins\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\..\detection-plugins\$(InputName).c.new" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Target
# End Project
