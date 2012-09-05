;NSIS Modern User Interface
;Basic Example Script
;Written by Joost Verburg

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------
;General

  ;Name and file
  Name "Links WWW Browser"
  ;Icon "links.ico"
  ;!define MUI_ICON "links.ico"
  OutFile "Links-install.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\Links"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\Links" ""

  RequestExecutionLevel admin

  Var MUI_TEMP
  Var STARTMENU_FOLDER

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "COPYING"
;  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY

!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Links" 
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Links"
  
!insertmacro MUI_PAGE_STARTMENU Application $STARTMENU_FOLDER

  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "-Default Links browser files" DefaultSection

  SetOutPath "$INSTDIR"
  
  ;ADD YOUR OWN FILES HERE...

File COPYING
File README
File links.exe
;File c:\cygwin\bin\cygbz2-1.dll
File c:\cygwin\bin\cygcrypto-0.9.8.dll
File c:\cygwin\bin\cygssl-0.9.8.dll
File c:\cygwin\bin\cygwin1.dll
;File c:\cygwin\bin\cygz.dll
  
  ;Store installation folder
  WriteRegStr HKCU "Software\Links" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

!insertmacro MUI_STARTMENU_WRITE_BEGIN Application

CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Links.lnk" "$INSTDIR\Links.exe"
CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
!insertmacro MUI_STARTMENU_WRITE_END
SectionEnd


;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...
Delete "$INSTDIR\BRAILLE_HOWTO"
Delete "$INSTDIR\COPYING"
Delete "$INSTDIR\KEYS"
Delete "$INSTDIR\README"
Delete "$INSTDIR\Links.exe"
Delete "$INSTDIR\cygbz2-1.dll"
Delete "$INSTDIR\cygcrypto-0.9.8.dll"
Delete "$INSTDIR\cygssl-0.9.8.dll"
Delete "$INSTDIR\cygwin1.dll"
Delete "$INSTDIR\cygz.dll"
Delete "$INSTDIR\.links\*"
RMDir "$INSTDIR\.links"

  Delete "$INSTDIR\Uninstall.exe"

  RMDir "$INSTDIR"

!insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP

Delete "$SMPROGRAMS\$MUI_TEMP\Uninstall.lnk"
Delete "$SMPROGRAMS\$MUI_TEMP\Links.lnk"
StrCpy $MUI_TEMP "$SMPROGRAMS\$MUI_TEMP"

startMenuDeleteLoop:
ClearErrors
RMDir $MUI_TEMP
GetFullPathName $MUI_TEMP "$MUI_TEMP\.."

IfErrors startMenuDeleteLoopDone

StrCmp $MUI_TEMP $SMPROGRAMS startMenuDeleteLoopDone startMenuDeleteLoop
startMenuDeleteLoopDone:

  DeleteRegKey /ifempty HKCU "Software\Links"

SectionEnd
