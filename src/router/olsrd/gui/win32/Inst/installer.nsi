;
;  The olsr.org Optimized Link-State Routing daemon (olsrd)
;  Copyright (c) 2004, Thomas Lopatic (thomas@lopatic.de)
;  All rights reserved.
;
;  Redistribution and use in source and binary forms, with or without 
;  modification, are permitted provided that the following conditions 
;  are met:
;
;  * Redistributions of source code must retain the above copyright 
;    notice, this list of conditions and the following disclaimer.
;  * Redistributions in binary form must reproduce the above copyright 
;    notice, this list of conditions and the following disclaimer in 
;    the documentation and/or other materials provided with the 
;    distribution.
;  * Neither the name of olsr.org, olsrd nor the names of its 
;    contributors may be used to endorse or promote products derived 
;    from this software without specific prior written permission.
;
;  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
;  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
;  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
;  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
;  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
;  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
;  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
;  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
;  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
;  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
;  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
;  POSSIBILITY OF SUCH DAMAGE.
;
;  Visit http://www.olsr.org for more information.
;
;  If you find this software useful feel free to make a donation
;  to the project. For more information see the website or contact
;  the copyright holders.
;
;

Name olsr.org
OutFile ..\..\..\olsr-setup.exe
BrandingText "www.olsr.org"
InstallDir $PROGRAMFILES\olsr.org
LicenseData ..\..\..\license.txt
XPStyle on

Page license
Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Function .onInit
        MessageBox MB_YESNO "This will install olsr.org 0.6.0 on your computer. Continue?" IDYES NoAbort
        Abort
NoAbort:
FunctionEnd

InstType "ETX Configuration (recommended)"
InstType "RFC Configuration"

Section "Program Files"

        SectionIn 1 2 RO

        SetOutPath $INSTDIR

        File ..\Main\release\Switch.exe
        File ..\Shim\release\Shim.exe
        File ..\..\..\olsrd.exe
        File ..\..\..\src\cfgparser\olsrd_cfgparser.dll
        File /oname=README.txt ..\..\..\README
        File /oname=README-LQ.html ..\..\..\README-Link-Quality.html
        File /oname=README-Fish-Eye.txt ..\..\..\README-Link-Quality-Fish-Eye.txt
        File ..\..\..\README-Olsr-Switch.html
        File linux-manual.txt
        File ..\Main\RFC-Default.olsr
        File ..\Main\LQ-Default.olsr
        File ..\..\..\lib\dot_draw\olsrd_dot_draw.dll
        File ..\..\..\lib\httpinfo\olsrd_httpinfo.dll
        File ..\..\..\lib\mini\olsrd_mini.dll
        File ..\..\..\lib\pgraph\olsrd_pgraph.dll
        File ..\..\..\lib\secure\olsrd_secure.dll
        File ..\..\..\lib\txtinfo\olsrd_txtinfo.dll

        WriteRegStr HKLM Software\Microsoft\Windows\CurrentVersion\Uninstall\olsr.org DisplayName olsr.org
        WriteRegStr HKLM Software\Microsoft\Windows\CurrentVersion\Uninstall\olsr.org UninstallString $INSTDIR\uninstall.exe

        WriteRegDWORD HKLM Software\Microsoft\Windows\CurrentVersion\Uninstall\olsr.org NoModify 1
        WriteRegDWORD HKLM Software\Microsoft\Windows\CurrentVersion\Uninstall\olsr.org NoRepair 1

        WriteUninstaller $INSTDIR\uninstall.exe

SectionEnd

Section "ETX Configuration"

        SectionIn 1 RO

        File ..\..\..\gui\win32\Main\RFC-Default.olsr
        File ..\..\..\gui\win32\Main\LQ-Default.olsr
        File /oname=Default.olsr ..\..\..\gui\win32\Main\LQ-Default.olsr

SectionEnd

Section "RFC Configuration"

        SectionIn 2 RO

        File ..\..\..\gui\win32\Main\RFC-Default.olsr
        File ..\..\..\gui\win32\Main\LQ-Default.olsr
        File /oname=Default.olsr ..\..\..\gui\win32\Main\RFC-Default.olsr

SectionEnd

Section "Start Menu Shortcuts"

        SectionIn 1 2

        CreateDirectory $SMPROGRAMS\olsr.org

        CreateShortCut "$SMPROGRAMS\olsr.org\OLSR Switch.lnk" $INSTDIR\Switch.exe "" $INSTDIR\Switch.exe 0
        CreateShortCut $SMPROGRAMS\olsr.org\README.lnk $INSTDIR\README.txt
        CreateShortCut $SMPROGRAMS\olsr.org\README-LQ.lnk $INSTDIR\README-LQ.html
        CreateShortCut $SMPROGRAMS\olsr.org\Uninstall.lnk $INSTDIR\uninstall.exe "" $INSTDIR\uninstall.exe 0

SectionEnd

Section "Desktop Shortcut"

        SectionIn 1 2

        CreateShortCut "$DESKTOP\OLSR Switch.lnk" $INSTDIR\Switch.exe "" $INSTDIR\Switch.exe 0

SectionEnd

Section "File Association (*.olsr)"

        SectionIn 1 2

        WriteRegStr HKCR .olsr "" OlsrOrgConfigFile

        WriteRegStr HKCR OlsrOrgConfigFile "" "olsr.org Configuration File"

        WriteRegStr HKCR OlsrOrgConfigFile\shell "" open
        WriteRegStr HKCR OlsrOrgConfigFile\DefaultIcon "" $INSTDIR\Switch.exe,0
        WriteRegStr HKCR OlsrOrgConfigFile\shell\open\command "" '$INSTDIR\Switch.exe "%1"'

SectionEnd

Section "Uninstall"

        DeleteRegKey HKLM Software\Microsoft\Windows\CurrentVersion\Uninstall\olsr.org

        DeleteRegKey HKCR .olsr
        DeleteRegKey HKCR OlsrOrgConfigFile

        Delete $INSTDIR\Switch.exe
        Delete $INSTDIR\Shim.exe
        Delete $INSTDIR\olsrd.exe
        Delete $INSTDIR\olsr_switch.exe
	Delete $INSTDIR\olsrd_cfgparser.dll
        Delete $INSTDIR\README.txt
        Delete $INSTDIR\README-LQ.html
        Delete $INSTDIR\README-Fish-Eye.txt
        Delete $INSTDIR\README-Olsr-Switch.html
        Delete $INSTDIR\linux-manual.txt
        Delete $INSTDIR\Default.olsr
        Delete $INSTDIR\RFC-Default.olsr
        Delete $INSTDIR\LQ-Default.olsr
	Delete $INSTDIR\olsrd.conf.rfc
	Delete $INSTDIR\olsrd.conf.lq
        Delete $INSTDIR\olsrd_dot_draw.dll
        Delete $INSTDIR\olsrd_httpinfo.dll
        Delete $INSTDIR\olsrd_mini.dll
        Delete $INSTDIR\olsrd_pgraph.dll
        Delete $INSTDIR\olsrd_secure.dll
        Delete $INSTDIR\olsrd_txtinfo.dll
        Delete $INSTDIR\uninstall.exe

        RMDir $INSTDIR

        Delete "$SMPROGRAMS\olsr.org\OLSR Switch.lnk"
        Delete $SMPROGRAMS\olsr.org\README.lnk
        Delete $SMPROGRAMS\olsr.org\README-LQ.lnk
        Delete $SMPROGRAMS\olsr.org\Uninstall.lnk

        RMDir $SMPROGRAMS\olsr.org

        Delete "$DESKTOP\OLSR Switch.lnk"

SectionEnd
