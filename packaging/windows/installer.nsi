; ZenRunner NSIS Installer Script
; This script creates a Windows installer for ZenRunner
; Requires NSIS 3.x or later

;--------------------------------
; Includes

!include "MUI2.nsh"
!include "FileFunc.nsh"
!include "x64.nsh"

;--------------------------------
; General Configuration

; Application name and version
!define APPNAME "ZenRunner"
!define COMPANYNAME "ZenRunner Project"
!define DESCRIPTION "A high-performance native process manager for developers"
!define VERSIONMAJOR 1
!define VERSIONMINOR 0
!define VERSIONBUILD 0

; Installer name
Name "${APPNAME}"
OutFile "..\..\build\ZenRunner-Setup-${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONBUILD}.exe"

; Default installation directory
InstallDir "$PROGRAMFILES64\${APPNAME}"

; Get installation directory from registry if available
InstallDirRegKey HKLM "Software\${APPNAME}" "InstallDir"

; Request application privileges (normal user - not admin)
RequestExecutionLevel user

; Branding
BrandingText "${APPNAME} v${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONBUILD}"

;--------------------------------
; Interface Settings

!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Welcome page
!define MUI_WELCOMEPAGE_TITLE "Welcome to ${APPNAME} Setup"
!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of ${APPNAME}.$\r$\n$\r$\n${DESCRIPTION}$\r$\n$\r$\nClick Next to continue."

; Finish page
!define MUI_FINISHPAGE_RUN "$INSTDIR\ZenRunner.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Launch ${APPNAME}"

;--------------------------------
; Pages

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\..\LICENSE.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
; Languages

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "French"

;--------------------------------
; Installer Sections

Section "Core Application" SecCore
    SectionIn RO ; Required section
    
    SetOutPath "$INSTDIR"
    
    ; Application files
    File "..\..\build\bin\Release\ZenRunner.exe"
    
    ; Qt runtime libraries
    File /nonfatal "..\..\build\bin\Release\Qt6Core.dll"
    File /nonfatal "..\..\build\bin\Release\Qt6Gui.dll"
    File /nonfatal "..\..\build\bin\Release\Qt6Quick.dll"
    File /nonfatal "..\..\build\bin\Release\Qt6Qml.dll"
    File /nonfatal "..\..\build\bin\Release\Qt6Widgets.dll"
    File /nonfatal "..\..\build\bin\Release\Qt6Network.dll"
    
    ; Qt plugins
    SetOutPath "$INSTDIR\platforms"
    File /nonfatal /r "..\..\build\bin\Release\platforms\*.*"
    
    SetOutPath "$INSTDIR\imageformats"
    File /nonfatal /r "..\..\build\bin\Release\imageformats\*.*"
    
    SetOutPath "$INSTDIR\styles"
    File /nonfatal /r "..\..\build\bin\Release\styles\*.*"
    
    SetOutPath "$INSTDIR\qmltooling"
    File /nonfatal /r "..\..\build\bin\Release\qmltooling\*.*"
    
    ; Documentation
    SetOutPath "$INSTDIR\docs"
    File /nonfatal /r "..\..\docs\*.md"
    File /nonfatal "..\..\README.md"
    File /nonfatal "..\..\INSTALL.md"
    
    ; Create uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"
    
    ; Write registry keys for Add/Remove Programs
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayName" "${APPNAME}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "UninstallString" "$\"$INSTDIR\Uninstall.exe$\""
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "QuietUninstallString" "$\"$INSTDIR\Uninstall.exe$\" /S"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "InstallLocation" "$INSTDIR"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "Publisher" "${COMPANYNAME}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayVersion" "${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONBUILD}"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "VersionMajor" ${VERSIONMAJOR}
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "VersionMinor" ${VERSIONMINOR}
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "NoRepair" 1
    
    ; Store installation directory
    WriteRegStr HKLM "Software\${APPNAME}" "InstallDir" "$INSTDIR"
    
    ; Get installed size
    ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
    IntFmt $0 "0x%08X" $0
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "EstimatedSize" "$0"
    
SectionEnd

Section "Start Menu Shortcuts" SecStartMenu
    CreateDirectory "$SMPROGRAMS\${APPNAME}"
    CreateShortcut "$SMPROGRAMS\${APPNAME}\${APPNAME}.lnk" "$INSTDIR\ZenRunner.exe"
    CreateShortcut "$SMPROGRAMS\${APPNAME}\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Desktop Shortcut" SecDesktop
    CreateShortcut "$DESKTOP\${APPNAME}.lnk" "$INSTDIR\ZenRunner.exe"
SectionEnd

Section /o "Add to PATH" SecPath
    ; Add installation directory to user PATH
    EnVar::AddValue "PATH" "$INSTDIR"
    Pop $0
    DetailPrint "Added to PATH: $0"
SectionEnd

;--------------------------------
; Section Descriptions

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} "Core application files (required)"
    !insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenu} "Create shortcuts in Start Menu"
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "Create shortcut on Desktop"
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPath} "Add ZenRunner to PATH environment variable"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
; Uninstaller Section

Section "Uninstall"
    ; Remove files
    Delete "$INSTDIR\ZenRunner.exe"
    Delete "$INSTDIR\*.dll"
    Delete "$INSTDIR\Uninstall.exe"
    
    ; Remove directories
    RMDir /r "$INSTDIR\platforms"
    RMDir /r "$INSTDIR\imageformats"
    RMDir /r "$INSTDIR\styles"
    RMDir /r "$INSTDIR\qmltooling"
    RMDir /r "$INSTDIR\docs"
    
    ; Remove installation directory if empty
    RMDir "$INSTDIR"
    
    ; Remove shortcuts
    Delete "$SMPROGRAMS\${APPNAME}\${APPNAME}.lnk"
    Delete "$SMPROGRAMS\${APPNAME}\Uninstall.lnk"
    RMDir "$SMPROGRAMS\${APPNAME}"
    Delete "$DESKTOP\${APPNAME}.lnk"
    
    ; Remove registry keys
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
    DeleteRegKey HKLM "Software\${APPNAME}"
    
    ; Remove from PATH if present
    EnVar::DeleteValue "PATH" "$INSTDIR"
    Pop $0
    
SectionEnd

;--------------------------------
; Functions

Function .onInit
    ; Check if already installed
    ReadRegStr $R0 HKLM "Software\${APPNAME}" "InstallDir"
    StrCmp $R0 "" done
    
    MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
    "${APPNAME} is already installed. $\n$\nClick 'OK' to remove the previous version or 'Cancel' to cancel this installation." \
    IDOK uninst
    Abort
    
uninst:
    ; Run the uninstaller
    ClearErrors
    ExecWait '$R0\Uninstall.exe _?=$R0'
    
done:
FunctionEnd

Function un.onInit
    MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to uninstall ${APPNAME}?" IDYES +2
    Abort
FunctionEnd
