# Microsoft Developer Studio Project File - Name="snort" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=snort - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "snort.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "snort.mak" CFG="snort - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "snort - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "snort - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "snort - Win32 Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "snort___Win32_Release0"
# PROP BASE Intermediate_Dir "snort___Win32_Release0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "snort___Win32_Release"
# PROP Intermediate_Dir "snort___Win32_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\..\dynamic-output\plugins" /I "..\Win32-Includes\zlib" /I "..\..\.." /I "..\.." /I "..\..\control" /I "..\..\sfutil" /I "..\..\target-based" /I "..\Win32-Includes" /I "..\Win32-Includes\libnet" /I "..\Win32-Includes\WinPCAP" /I "..\..\output-plugins" /I "..\..\detection-plugins" /I "..\..\dynamic-plugins" /I "..\..\preprocessors" /I "..\..\preprocessors\portscan" /I "..\..\preprocessors\HttpInspect\Include" /I "..\..\preprocessors\Stream5" /I "..\..\..\daq\api" /I "..\..\..\daq\sfbpf" /I "..\Win32-Includes\libdnet" /D "NDEBUG" /D "OUTPUT_DLL" /D "_CONSOLE" /D __BEGIN_DECLS="" /D __END_DECLS="" /D "ENABLE_RESPOND" /D "ENABLE_WIN32_SERVICE" /D "ZLIB" /D "ENABLE_RESPONSE3" /D "_WINDOWS" /D "_USRDLL" /D "ACTIVE_RESPONSE" /D "ENABLE_REACT" /D "GRE" /D "MPLS" /D "TARGET_BASED" /D "PERF_PROFILING" /D "ENABLE_PAF" /D "WIN32" /D "_MBCS" /D "HAVE_CONFIG_H" /D "_AFXDLL" /D SIGNAL_SNORT_READ_ATTR_TBL=30 /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\dynamic-output\plugins" /I "..\Win32-Includes\zlib" /I "..\..\.." /I "..\.." /I "..\..\control" /I "..\..\sfutil" /I "..\..\target-based" /I "..\Win32-Includes" /I "..\Win32-Includes\libnet" /I "..\Win32-Includes\WinPCAP" /I "..\..\output-plugins" /I "..\..\detection-plugins" /I "..\..\dynamic-plugins" /I "..\..\preprocessors" /I "..\..\preprocessors\portscan" /I "..\..\preprocessors\HttpInspect\Include" /I "..\..\preprocessors\Stream6" /I "..\..\preprocessors\Session" /I "..\..\..\daq\api" /I "..\..\..\daq\sfbpf" /I "..\Win32-Includes\libdnet" /I "..\..\file-process" /I "..\..\file-process\libs" /I "..\..\side-channel" /D "NDEBUG" /D "OUTPUT_DLL" /D "_CONSOLE" /D __BEGIN_DECLS="" /D __END_DECLS="" /D "ENABLE_RESPOND" /D "ENABLE_WIN32_SERVICE" /D "ZLIB" /D "ENABLE_RESPONSE3" /D "_WINDOWS" /D "_USRDLL" /D "ACTIVE_RESPONSE" /D "ENABLE_REACT" /D "GRE" /D "MPLS" /D "TARGET_BASED" /D "PERF_PROFILING" /D "ENABLE_PAF" /D "WIN32" /D "_MBCS" /D "HAVE_CONFIG_H" /D "_AFXDLL" /D SIGNAL_SNORT_READ_ATTR_TBL=30 /D "NORMALIZER" /D BYTE_ORDER=LITTLE_ENDIAN /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 zlib.lib dnet.lib daq.lib Packet.lib iphlpapi.lib wsock32.lib advapi32.lib pcre.lib wpcap.lib /nologo /subsystem:console /machine:I386 /out:"snort___Win32_Release/snort.exe" /libpath:"..\..\..\daq\Release" /libpath:"..\Win32-Libraries" /libpath:"..\Win32-Libraries\libdnet"
# SUBTRACT BASE LINK32 /incremental:yes
# ADD LINK32 zlib.lib dnet.lib daq.lib Packet.lib iphlpapi.lib wsock32.lib advapi32.lib pcre.lib wpcap.lib /nologo /subsystem:console /machine:I386 /libpath:"..\..\..\daq\Release" /libpath:"..\Win32-Libraries" /libpath:"..\Win32-Libraries\libdnet"
# SUBTRACT LINK32 /incremental:yes

!ELSEIF  "$(CFG)" == "snort - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "snort___Win32_Debug0"
# PROP BASE Intermediate_Dir "snort___Win32_Debug0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "snort___Win32_Debug"
# PROP Intermediate_Dir "snort___Win32_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\dynamic-output\plugins" /I "..\Win32-Includes\mysql" /I "..\Win32-Includes\zlib" /I "..\..\.." /I "..\.." /I "..\..\control" /I "..\..\sfutil" /I "..\..\target-based" /I "..\Win32-Includes" /I "..\Win32-Includes\libnet" /I "..\Win32-Includes\WinPCAP" /I "..\..\output-plugins" /I "..\..\detection-plugins" /I "..\..\dynamic-plugins" /I "..\..\preprocessors" /I "..\..\preprocessors\portscan" /I "..\..\preprocessors\HttpInspect\Include" /I "..\..\preprocessors\Stream5" /I "..\..\..\daq\api" /I "..\..\..\daq\sfbpf" /I "..\Win32-Includes\libdnet" /D "_DEBUG" /D "DEBUG" /D "OUTPUT_DLL" /D "_CONSOLE" /D __BEGIN_DECLS="" /D __END_DECLS="" /D "ENABLE_RESPOND" /D "ENABLE_WIN32_SERVICE" /D "ZLIB" /D "ENABLE_RESPONSE3" /D "_WINDOWS" /D "_USRDLL" /D "ACTIVE_RESPONSE" /D "ENABLE_REACT" /D "GRE" /D "MPLS" /D "TARGET_BASED" /D "PERF_PROFILING" /D "ENABLE_PAF" /D "WIN32" /D "_MBCS" /D "HAVE_CONFIG_H" /D "_AFXDLL" /D SIGNAL_SNORT_READ_ATTR_TBL=30 /FR /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\dynamic-output\plugins" /I "..\Win32-Includes\zlib" /I "..\..\.." /I "..\.." /I "..\..\control" /I "..\..\sfutil" /I "..\..\target-based" /I "..\Win32-Includes" /I "..\Win32-Includes\libnet" /I "..\Win32-Includes\WinPCAP" /I "..\..\output-plugins" /I "..\..\detection-plugins" /I "..\..\dynamic-plugins" /I "..\..\preprocessors" /I "..\..\preprocessors\portscan" /I "..\..\preprocessors\HttpInspect\Include" /I "..\..\preprocessors\Stream6" /I "..\..\preprocessors\Session" /I "..\..\..\daq\api" /I "..\..\..\daq\sfbpf" /I "..\Win32-Includes\libdnet" /I "..\..\file-process" /I "..\..\file-process\libs" /I "..\..\side-channel" /D "_DEBUG" /D "DEBUG" /D "OUTPUT_DLL" /D "_CONSOLE" /D __BEGIN_DECLS="" /D __END_DECLS="" /D "ENABLE_RESPOND" /D "ENABLE_WIN32_SERVICE" /D "ZLIB" /D "ENABLE_RESPONSE3" /D "_WINDOWS" /D "_USRDLL" /D "ACTIVE_RESPONSE" /D "ENABLE_REACT" /D "GRE" /D "MPLS" /D "TARGET_BASED" /D "PERF_PROFILING" /D "ENABLE_PAF" /D "WIN32" /D "_MBCS" /D "HAVE_CONFIG_H" /D "_AFXDLL" /D SIGNAL_SNORT_READ_ATTR_TBL=30 /D "NORMALIZER" /D BYTE_ORDER=LITTLE_ENDIAN /FR /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 zlib.lib dnet.lib daq.lib Packet.lib iphlpapi.lib wsock32.lib advapi32.lib pcre.lib wpcap.lib /nologo /subsystem:console /debug /machine:I386 /out:"snort___Win32_Debug/snort.exe" /pdbtype:sept /libpath:"..\..\..\daq\Debug" /libpath:"..\Win32-Libraries" /libpath:"..\Win32-Libraries\libdnet"
# ADD LINK32 zlib.lib dnet.lib daq.lib Packet.lib iphlpapi.lib wsock32.lib advapi32.lib pcre.lib wpcap.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"..\..\..\daq\Debug" /libpath:"..\Win32-Libraries" /libpath:"..\Win32-Libraries\libdnet"

!ENDIF 

# Begin Target

# Name "snort - Win32 Release"
# Name "snort - Win32 Debug"
# Begin Group "Source"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Detection Plugins"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\detection-plugins\detection_options.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\detection_options.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sf_snort_plugin_hdropts.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_asn1.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_asn1.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_asn1_detect.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_asn1_detect.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_base64_data.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_base64_data.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_base64_decode.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_base64_decode.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_byte_check.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_byte_check.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_byte_math.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_byte_math.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_byte_extract.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_byte_extract.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_byte_jump.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_byte_jump.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_clientserver.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_clientserver.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_cvs.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_cvs.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_dsize_check.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_dsize_check.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_file_data.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_file_data.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_file_type.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_file_type.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_flowbits.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_flowbits.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_ftpbounce.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_ftpbounce.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_hdr_opt_wrap.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_hdr_opt_wrap.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_icmp_code_check.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_icmp_code_check.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_icmp_id_check.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_icmp_id_check.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_icmp_seq_check.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_icmp_seq_check.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_icmp_type_check.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_icmp_type_check.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_ip_fragbits.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_ip_fragbits.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_ip_id_check.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_ip_id_check.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_ip_proto.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_ip_proto.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_ip_same_check.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_ip_same_check.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_ip_tos_check.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_ip_tos_check.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_ipoption_check.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_ipoption_check.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_isdataat.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_isdataat.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_pattern_match.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_pattern_match.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_pcre.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_pcre.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_pkt_data.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_pkt_data.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_react.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_react.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_replace.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_replace.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_respond.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_respond3.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_rpc_check.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_rpc_check.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_session.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_session.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_tcp_ack_check.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_tcp_ack_check.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_tcp_flag_check.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_tcp_flag_check.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_tcp_seq_check.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_tcp_seq_check.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_tcp_win_check.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_tcp_win_check.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_ttl_check.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_ttl_check.h"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_urilen_check.c"
# End Source File
# Begin Source File

SOURCE="..\..\detection-plugins\sp_urilen_check.h"
# End Source File
# End Group
# Begin Group "Output Plugins"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\output-plugins\spo_alert_fast.c"
# End Source File
# Begin Source File

SOURCE="..\..\output-plugins\spo_alert_fast.h"
# End Source File
# Begin Source File

SOURCE="..\..\output-plugins\spo_alert_full.c"
# End Source File
# Begin Source File

SOURCE="..\..\output-plugins\spo_alert_full.h"
# End Source File
# Begin Source File

SOURCE="..\..\output-plugins\spo_alert_sf_socket.c"
# End Source File
# Begin Source File

SOURCE="..\..\output-plugins\spo_alert_sf_socket.h"
# End Source File
# Begin Source File

SOURCE="..\..\output-plugins\spo_alert_syslog.c"
# End Source File
# Begin Source File

SOURCE="..\..\output-plugins\spo_alert_syslog.h"
# End Source File
# Begin Source File

SOURCE="..\..\output-plugins\spo_alert_test.c"
# End Source File
# Begin Source File

SOURCE="..\..\output-plugins\spo_alert_test.h"
# End Source File
# Begin Source File

SOURCE="..\..\output-plugins\spo_alert_unixsock.c"
# End Source File
# Begin Source File

SOURCE="..\..\output-plugins\spo_alert_unixsock.h"
# End Source File
# Begin Source File

SOURCE="..\..\output-plugins\spo_csv.c"
# End Source File
# Begin Source File

SOURCE="..\..\output-plugins\spo_csv.h"
# End Source File
# Begin Source File

SOURCE="..\..\output-plugins\spo_log_ascii.c"
# End Source File
# Begin Source File

SOURCE="..\..\output-plugins\spo_log_ascii.h"
# End Source File
# Begin Source File

SOURCE="..\..\output-plugins\spo_log_null.c"
# End Source File
# Begin Source File

SOURCE="..\..\output-plugins\spo_log_null.h"
# End Source File
# Begin Source File

SOURCE="..\..\output-plugins\spo_log_tcpdump.c"
# End Source File
# Begin Source File

SOURCE="..\..\output-plugins\spo_log_tcpdump.h"
# End Source File
# Begin Source File

SOURCE="..\..\output-plugins\spo_unified2.c"
# End Source File
# Begin Source File

SOURCE="..\..\output-plugins\spo_unified2.h"
# End Source File
# End Group
# Begin Group "Parser"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\parser\IpAddrSet.c
# End Source File
# Begin Source File

SOURCE=..\..\parser\IpAddrSet.h
# End Source File
# End Group
# Begin Group "Preprocessors"

# PROP Default_Filter ""
# Begin Group "HttpInspect"

# PROP Default_Filter ""
# Begin Group "Anomaly Detection"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\anomaly_detection\hi_ad.c
# End Source File
# End Group
# Begin Group "Client"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\client\hi_client.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\client\hi_client_norm.c
# End Source File
# End Group
# Begin Group "Event Output"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\event_output\hi_eo_log.c
# End Source File
# End Group
# Begin Group "Include"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\include\file_decomp.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\include\file_decomp_PDF.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\include\file_decomp_SWF.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\include\hi_ad.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\include\hi_client.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\include\hi_client_norm.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\include\hi_cmd_lookup.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\include\hi_eo.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\include\hi_eo_events.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\include\hi_eo_log.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\include\hi_include.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\include\hi_mi.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\include\hi_norm.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\include\hi_paf.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\include\hi_return_codes.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\include\hi_server.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\include\hi_server_norm.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\include\hi_si.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\include\hi_ui_config.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\include\hi_ui_iis_unicode_map.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\include\hi_ui_server_lookup.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\include\hi_util.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\include\hi_util_hbm.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\include\hi_util_kmap.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\include\hi_util_xmalloc.h
# End Source File
# End Group
# Begin Group "Mode Inspection"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\mode_inspection\hi_mi.c
# End Source File
# End Group
# Begin Group "Normalization"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\normalization\hi_norm.c
# End Source File
# End Group
# Begin Group "Server"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\server\hi_server.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\server\hi_server_norm.c
# End Source File
# End Group
# Begin Group "Session Inspection"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\session_inspection\hi_si.c
# End Source File
# End Group
# Begin Group "User Interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\user_interface\hi_ui_config.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\user_interface\hi_ui_iis_unicode_map.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\user_interface\hi_ui_server_lookup.c
# End Source File
# End Group
# Begin Group "Utils"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\utils\hi_cmd_lookup.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\utils\hi_paf.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\utils\hi_util_hbm.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\utils\hi_util_kmap.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\utils\hi_util_xmalloc.c
# End Source File
# End Group
# Begin Group "files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\files\file_decomp.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\files\file_decomp_PDF.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\HttpInspect\files\file_decomp_SWF.c
# End Source File
# End Group
# End Group
# Begin Group "Stream6"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\preprocessors\Stream6\snort_stream_icmp.c

# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\Stream6\snort_stream_icmp.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\Stream6\snort_stream_ip.c

# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\Stream6\snort_stream_ip.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\Stream6\snort_stream_tcp.c

# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\Stream6\snort_stream_tcp.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\Stream6\snort_stream_udp.c

# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\Stream6\snort_stream_udp.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\Stream6\stream_common.c

# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\Stream6\stream_common.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\Stream6\stream_paf.c

# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\Stream6\stream_paf.h
# End Source File
# End Group
# Begin Group "Session"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\preprocessors\Session\session_common.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\Session\session_common.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\Session\session_expect.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\Session\session_expect.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\Session\snort_session.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\Session\snort_session.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\preprocessors\normalize.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\normalize.h
# End Source File
# Begin Source File

SOURCE="..\..\preprocessors\perf-base.c"
# End Source File
# Begin Source File

SOURCE="..\..\preprocessors\perf-base.h"
# End Source File
# Begin Source File

SOURCE="..\..\preprocessors\perf-event.c"
# End Source File
# Begin Source File

SOURCE="..\..\preprocessors\perf-event.h"
# End Source File
# Begin Source File

SOURCE="..\..\preprocessors\perf-flow.c"
# End Source File
# Begin Source File

SOURCE="..\..\preprocessors\perf-flow.h"
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\perf.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\perf.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\perf_indicators.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\perf_indicators.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\portscan.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\portscan.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\session_api.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\session_api.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\sfprocpidstats.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\sfprocpidstats.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\snort_httpinspect.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\snort_httpinspect.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\spp_arpspoof.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\spp_arpspoof.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\spp_bo.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\spp_bo.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\spp_frag3.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\spp_frag3.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\spp_httpinspect.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\spp_httpinspect.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\spp_normalize.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\spp_normalize.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\spp_perfmonitor.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\spp_perfmonitor.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\spp_rpc_decode.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\spp_rpc_decode.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\spp_session.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\spp_session.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\spp_sfportscan.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\spp_sfportscan.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\spp_stream6.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\spp_stream6.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\str_search.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\str_search.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\stream_api.c
# End Source File
# Begin Source File

SOURCE=..\..\preprocessors\stream_api.h
# End Source File
# End Group
# Begin Group "SFUtil"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\sfutil\acsmx.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\acsmx.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\acsmx2.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\acsmx2.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\asn1.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\asn1.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\bitop.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\bitop_funcs.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\bnfa_search.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\bnfa_search.h
# End Source File
# Begin Source File

SOURCE=..\..\event_queue.c
# End Source File
# Begin Source File

SOURCE=..\..\event_queue.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\getopt.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\getopt1.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\getopt_long.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\ipobj.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\ipobj.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\md5.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\md5.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\mpse.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\mpse.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_base64decode.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_base64decode.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_email_attach_decode.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_email_attach_decode.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_ip.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_ip.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_iph.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_ipvar.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_ipvar.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_sechash.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_sechash.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_seqnums.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_textlog.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_textlog.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_vartable.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sf_vartable.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfActionQueue.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfActionQueue.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfeventq.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfeventq.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfghash.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfghash.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfhashfcn.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfhashfcn.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfksearch.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfksearch.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sflsq.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sflsq.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfmemcap.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfmemcap.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfPolicy.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfPolicy.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfPolicyData.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfPolicyUserData.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfPolicyUserData.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfportobject.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfportobject.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfprimetable.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfprimetable.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfrf.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfrf.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfrim.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfrim.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfrt.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfrt.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfrt_dir.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfrt_dir.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfrt_trie.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfsnprintfappend.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfsnprintfappend.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfthd.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfthd.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfxhash.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sfxhash.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sha2.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\sha2.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\strvec.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\strvec.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\Unified2_common.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\util_jsnorm.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\util_jsnorm.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\util_math.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\util_math.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\util_net.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\util_net.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\util_str.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\util_str.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\util_unfold.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\util_unfold.h
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\util_utf.c
# End Source File
# Begin Source File

SOURCE=..\..\sfutil\util_utf.h
# End Source File
# End Group
# Begin Group "Dynamic Plugins"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_convert_dynamic.c"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_convert_dynamic.h"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_dynamic_common.h"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_dynamic_detection.h"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_dynamic_engine.h"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_dynamic_meta.h"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_dynamic_plugins.c"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_dynamic_preprocessor.h"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_dynamic_decompression.c"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sf_dynamic_decompression.h"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sp_dynamic.c"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sp_dynamic.h"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sp_preprocopt.c"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-plugins\sp_preprocopt.h"
# End Source File
# End Group
# Begin Group "Target-Based"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\target-based\sf_attribute_table.c"
# End Source File
# Begin Source File

SOURCE="..\..\target-based\sf_attribute_table.h"
# End Source File
# Begin Source File

SOURCE="..\..\target-based\sf_attribute_table.y"

!IF  "$(CFG)" == "snort - Win32 Release"

# Begin Custom Build
InputPath="..\..\target-based\sf_attribute_table.y"
InputName=sf_attribute_table

BuildCmds= \
	c:\cygwin\bin\bison -d -psfat_ -o../../target-based/$(InputName).c ../../target-based/$(InputName).y

"..\..\target-based\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\..\target-based\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "snort - Win32 Debug"

# Begin Custom Build
InputPath="..\..\target-based\sf_attribute_table.y"
InputName=sf_attribute_table

BuildCmds= \
	c:\cygwin\bin\bison -d -psfat_ -o../../target-based/$(InputName).c ../../target-based/$(InputName).y

"..\..\target-based\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\..\target-based\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\target-based\sf_attribute_table_parser.c"
# End Source File
# Begin Source File

SOURCE="..\..\target-based\sf_attribute_table_parser.l"

!IF  "$(CFG)" == "snort - Win32 Release"

# Begin Custom Build
InputPath="..\..\target-based\sf_attribute_table_parser.l"
InputName=sf_attribute_table_parser

"..\..\target-based\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	c:\cygwin\bin\flex -i -Psfat -o../../target-based/$(InputName).c ../../target-based/$(InputName).l

# End Custom Build

!ELSEIF  "$(CFG)" == "snort - Win32 Debug"

# Begin Custom Build
InputPath="..\..\target-based\sf_attribute_table_parser.l"
InputName=sf_attribute_table_parser

"..\..\target-based\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	c:\cygwin\bin\flex -i -Psfat -o../../target-based/$(InputName).c ../../target-based/$(InputName).l

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\target-based\sftarget_hostentry.c"
# End Source File
# Begin Source File

SOURCE="..\..\target-based\sftarget_hostentry.h"
# End Source File
# Begin Source File

SOURCE="..\..\target-based\sftarget_protocol_reference.c"
# End Source File
# Begin Source File

SOURCE="..\..\target-based\sftarget_protocol_reference.h"
# End Source File
# Begin Source File

SOURCE="..\..\target-based\sftarget_reader.c"
# End Source File
# Begin Source File

SOURCE="..\..\target-based\sftarget_reader.h"
# End Source File
# End Group
# Begin Group "Control"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\control\sfcontrol.c
# End Source File
# Begin Source File

SOURCE=..\..\control\sfcontrol.h
# End Source File
# Begin Source File

SOURCE=..\..\control\sfcontrol_funcs.h
# End Source File
# End Group
# Begin Group "Dynamic Output"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\dynamic-output\plugins\output.h"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-output\plugins\output_api.h"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-output\plugins\output_base.c"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-output\plugins\output_common.h"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-output\libs\output_lib.c"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-output\plugins\output_lib.h"
# End Source File
# Begin Source File

SOURCE="..\..\dynamic-output\plugins\output_plugin.c"
# ADD BASE CPP /D inline=__inline
# ADD CPP /D inline=__inline
# End Source File
# End Group
# Begin Group "File"

# PROP Default_Filter ""
# Begin Group "Lib"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\file-process\libs\file_config.c"
# End Source File
# Begin Source File

SOURCE="..\..\file-process\libs\file_config.h"
# End Source File
# Begin Source File

SOURCE="..\..\file-process\libs\file_identifier.c"
# End Source File
# Begin Source File

SOURCE="..\..\file-process\libs\file_identifier.h"
# End Source File
# Begin Source File

SOURCE="..\..\file-process\libs\file_lib.c"
# End Source File
# Begin Source File

SOURCE="..\..\file-process\libs\file_lib.h"
# End Source File
# End Group
# Begin Source File

SOURCE="..\..\file-process\circular_buffer.c"
# End Source File
# Begin Source File

SOURCE="..\..\file-process\circular_buffer.h"
# End Source File
# Begin Source File

SOURCE="..\..\file-process\file_api.h"
# End Source File
# Begin Source File

SOURCE="..\..\file-process\file_capture.c"
# End Source File
# Begin Source File

SOURCE="..\..\file-process\file_capture.h"
# End Source File
# Begin Source File

SOURCE="..\..\file-process\file_mempool.c"
# End Source File
# Begin Source File

SOURCE="..\..\file-process\file_mempool.h"
# End Source File
# Begin Source File

SOURCE="..\..\file-process\file_segment_process.c"
# End Source File
# Begin Source File

SOURCE="..\..\file-process\file_segment_process.h"
# End Source File
# Begin Source File

SOURCE="..\..\file-process\file_mime_config.c"
# End Source File
# Begin Source File

SOURCE="..\..\file-process\file_mime_config.h"
# End Source File
# Begin Source File

SOURCE="..\..\file-process\file_mime_process.c"
# End Source File
# Begin Source File

SOURCE="..\..\file-process\file_mime_process.h"
# End Source File
# Begin Source File

SOURCE="..\..\file-process\file_resume_block.c"
# End Source File
# Begin Source File

SOURCE="..\..\file-process\file_resume_block.h"
# End Source File
# Begin Source File

SOURCE="..\..\file-process\file_service.c"
# End Source File
# Begin Source File

SOURCE="..\..\file-process\file_service.h"
# End Source File
# Begin Source File

SOURCE="..\..\file-process\file_service_config.c"
# End Source File
# Begin Source File

SOURCE="..\..\file-process\file_service_config.h"
# End Source File
# Begin Source File

SOURCE="..\..\file-process\file_stats.c"
# End Source File
# Begin Source File

SOURCE="..\..\file-process\file_stats.h"
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\active.c
# End Source File
# Begin Source File

SOURCE=..\..\active.h
# End Source File
# Begin Source File

SOURCE=..\..\bounds.h
# End Source File
# Begin Source File

SOURCE=..\..\byte_extract.c
# End Source File
# Begin Source File

SOURCE=..\..\byte_extract.h
# End Source File
# Begin Source File

SOURCE=..\..\cdefs.h
# End Source File
# Begin Source File

SOURCE=..\..\checksum.h
# End Source File
# Begin Source File

SOURCE=..\..\debug.c
# End Source File
# Begin Source File

SOURCE=..\..\debug.h
# End Source File
# Begin Source File

SOURCE=..\..\decode.c
# End Source File
# Begin Source File

SOURCE=..\..\decode.h
# End Source File
# Begin Source File

SOURCE=..\..\detect.c

!IF  "$(CFG)" == "snort - Win32 Release"

# ADD CPP /I "..\..\preprocessors\Stream6" /I "..\..\preprocessors\Session"

!ELSEIF  "$(CFG)" == "snort - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\detect.h
# End Source File
# Begin Source File

SOURCE=..\..\detection_filter.c
# End Source File
# Begin Source File

SOURCE=..\..\detection_filter.h
# End Source File
# Begin Source File

SOURCE=..\..\detection_util.c
# End Source File
# Begin Source File

SOURCE=..\..\detection_util.h
# End Source File
# Begin Source File

SOURCE=..\..\encode.c
# End Source File
# Begin Source File

SOURCE=..\..\encode.h
# End Source File
# Begin Source File

SOURCE=..\..\event.h
# End Source File
# Begin Source File

SOURCE=..\..\event_wrapper.c
# End Source File
# Begin Source File

SOURCE=..\..\event_wrapper.h
# End Source File
# Begin Source File

SOURCE=..\..\fpcreate.c
# End Source File
# Begin Source File

SOURCE=..\..\fpcreate.h
# End Source File
# Begin Source File

SOURCE=..\..\fpdetect.c
# End Source File
# Begin Source File

SOURCE=..\..\fpdetect.h
# End Source File
# Begin Source File

SOURCE=..\..\generators.h
# End Source File
# Begin Source File

SOURCE=..\..\hashstring.c
# End Source File
# Begin Source File

SOURCE=..\..\hashstring.h
# End Source File
# Begin Source File

SOURCE=..\..\idle_processing.c
# End Source File
# Begin Source File

SOURCE=..\..\idle_processing.h
# End Source File
# Begin Source File

SOURCE=..\..\idle_processing_funcs.h
# End Source File
# Begin Source File

SOURCE=..\..\ipv6_port.h
# End Source File
# Begin Source File

SOURCE=..\..\log.c
# End Source File
# Begin Source File

SOURCE=..\..\log.h
# End Source File
# Begin Source File

SOURCE=..\..\log_text.c
# End Source File
# Begin Source File

SOURCE=..\..\log_text.h
# End Source File
# Begin Source File

SOURCE=..\..\mempool.c
# End Source File
# Begin Source File

SOURCE=..\..\mempool.h
# End Source File
# Begin Source File

SOURCE=..\..\mstring.c
# End Source File
# Begin Source File

SOURCE=..\..\mstring.h
# End Source File
# Begin Source File

SOURCE=..\..\obfuscation.c
# End Source File
# Begin Source File

SOURCE=..\..\obfuscation.h
# End Source File
# Begin Source File

SOURCE=..\..\packet_time.c
# End Source File
# Begin Source File

SOURCE=..\..\packet_time.h
# End Source File
# Begin Source File

SOURCE=..\..\parser.c
# End Source File
# Begin Source File

SOURCE=..\..\parser.h
# End Source File
# Begin Source File

SOURCE=..\..\pcap_pkthdr32.h
# End Source File
# Begin Source File

SOURCE=..\..\pcrm.c
# End Source File
# Begin Source File

SOURCE=..\..\pcrm.h
# End Source File
# Begin Source File

SOURCE=..\..\plugbase.c
# End Source File
# Begin Source File

SOURCE=..\..\plugbase.h
# End Source File
# Begin Source File

SOURCE=..\..\plugin_enum.h
# End Source File
# Begin Source File

SOURCE=..\..\preprocids.h
# End Source File
# Begin Source File

SOURCE=..\..\profiler.c
# End Source File
# Begin Source File

SOURCE=..\..\profiler.h
# End Source File
# Begin Source File

SOURCE=..\..\protocols.h
# End Source File
# Begin Source File

SOURCE=..\..\rate_filter.c
# End Source File
# Begin Source File

SOURCE=..\..\rate_filter.h
# End Source File
# Begin Source File

SOURCE=..\..\rules.h
# End Source File
# Begin Source File

SOURCE=..\..\sf_sdlist.c
# End Source File
# Begin Source File

SOURCE=..\..\sf_sdlist.h
# End Source File
# Begin Source File

SOURCE=..\..\sf_sdlist_types.h
# End Source File
# Begin Source File

SOURCE=..\..\sf_types.h
# End Source File
# Begin Source File

SOURCE=..\..\sfdaq.c
# End Source File
# Begin Source File

SOURCE=..\..\sfdaq.h
# End Source File
# Begin Source File

SOURCE=..\..\sfthreshold.c
# End Source File
# Begin Source File

SOURCE=..\..\sfthreshold.h
# End Source File
# Begin Source File

SOURCE=..\..\signature.c
# End Source File
# Begin Source File

SOURCE=..\..\signature.h
# End Source File
# Begin Source File

SOURCE=..\..\smalloc.h
# End Source File
# Begin Source File

SOURCE=..\..\snort.c
# End Source File
# Begin Source File

SOURCE=..\..\snort.h
# End Source File
# Begin Source File

SOURCE=..\..\snprintf.c
# End Source File
# Begin Source File

SOURCE=..\..\snprintf.h
# End Source File
# Begin Source File

SOURCE=..\..\spo_plugbase.h
# End Source File
# Begin Source File

SOURCE=..\..\strlcatu.c
# End Source File
# Begin Source File

SOURCE=..\..\strlcatu.h
# End Source File
# Begin Source File

SOURCE=..\..\strlcpyu.c
# End Source File
# Begin Source File

SOURCE=..\..\strlcpyu.h
# End Source File
# Begin Source File

SOURCE=..\..\tag.c
# End Source File
# Begin Source File

SOURCE=..\..\tag.h
# End Source File
# Begin Source File

SOURCE=..\..\timersub.h
# End Source File
# Begin Source File

SOURCE=..\..\util.c
# End Source File
# Begin Source File

SOURCE=..\..\util.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE="..\WIN32-Code\MSG00001.bin"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Code\name.mc"
# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE="..\WIN32-Code\name.rc"
# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# End Source File
# End Group
# Begin Group "Win32"

# PROP Default_Filter ""
# Begin Group "WinPCAP"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\WIN32-Includes\WinPCAP\bittypes.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\WinPCAP\bucket_lookup.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\WinPCAP\count_packets.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\WinPCAP\Devioctl.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\WinPCAP\Gnuc.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\WinPCAP\ip6_misc.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\WinPCAP\memory_t.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\WinPCAP\normal_lookup.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\WinPCAP\Ntddndis.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\WinPCAP\Ntddpack.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\WinPCAP\Packet32.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\WinPCAP\pcap-bpf.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\WinPCAP\pcap-int.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\WinPCAP\pcap-stdinc.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\WinPCAP\pcap.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\WinPCAP\pthread.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\WinPCAP\remote-ext.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\WinPCAP\sched.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\WinPCAP\semaphore.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\WinPCAP\tcp_session.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\WinPCAP\time_calls.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\WinPCAP\tme.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\WinPCAP\Win32-Extensions.h"
# End Source File
# End Group
# Begin Group "ZLib"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\WIN32-Includes\zlib\zconf.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\zlib\zlib.h"
# End Source File
# End Group
# Begin Source File

SOURCE="..\WIN32-Includes\rpc\auth.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\NET\Bpf.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\rpc\clnt.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\config.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Code\getopt.c"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\getopt.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\getopt1.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\gnuc.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Code\inet_aton.c"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Code\inet_pton.c"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\inttypes.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Code\misc.c"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Code\name.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\pcre.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\pcreposix.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\rpc\rpc_msg.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\stdint.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Code\strtok_r.c"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Code\syslog.c"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\syslog.h"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\UNISTD.H"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Code\win32_service.c"
# End Source File
# Begin Source File

SOURCE="..\WIN32-Includes\rpc\xdr.h"
# End Source File
# End Group
# End Target
# End Project
