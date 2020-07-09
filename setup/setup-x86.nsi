; example2.nsi
;
; This script is based on example1.nsi, but it remember the directory, 
; has uninstall support and (optionally) installs start menu shortcuts.
;
; It will install example2.nsi into a directory that the user selects,

;--------------------------------

!include MUI2.nsh

!define PRODUCT_NAME "Open Mod Manager"
!define PRODUCT_VERSION "0.9.0"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "..\include\gui\res\bitmap\inst_icon.ico"
!define MUI_UNICON "..\include\gui\res\bitmap\unin_icon.ico"


; The name of the installer
Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"

; The file to write
OutFile "OpenModMan_0-9-0-x86_setup.exe"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

; Build Unicode installer
Unicode True

; The default installation directory
InstallDir "$PROGRAMFILES\Open Mod Manager"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Open_Mod_Manager" "Install_Dir"

; Sets the text that is shown at the bottom of the install window
BrandingText " "

;LicenseText "..\LICENSE.TXT"
LicenseData "..\LICENSE.TXT"

; Sets the background color of the license data.
LicenseBkColor /windows

; Use modern Windows theme
XPStyle on

ShowInstDetails show
ShowUnInstDetails show

!define MUI_FINISHPAGE_RUN "$INSTDIR\OpenModMan.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Start Open Mod Manager"

;--------------------------------

; Pages
;Page license setBrandingImage
;Page components
;Page directory
;Page instfiles

!insertmacro MUI_PAGE_LICENSE "..\LICENSE.TXT"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

;--------------------------------

; The stuff to install
Section "OpenModMan (required)"

  SectionIn RO

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

  ; Put file there
  File "..\bin\32-bit\Release\OpenModMan.exe"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\Open_Mod_Manager "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Open_Mod_Manager" "DisplayName" "NSIS Example2"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Open_Mod_Manager" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Open_Mod_Manager" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Open_Mod_Manager" "NoRepair" 1
  WriteUninstaller "$INSTDIR\uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\Open Mod Manager"
  CreateShortcut "$SMPROGRAMS\Open Mod Manager\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortcut "$SMPROGRAMS\Open Mod Manager\Open Mod Manager.lnk" "$INSTDIR\OpenModMan.exe" "" "$INSTDIR\OpenModMan.exe" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Open_Mod_Manager"
  DeleteRegKey HKLM SOFTWARE\OpenModManager

  ; Remove files and uninstaller
  Delete $INSTDIR\*.*

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Open Mod Manager\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\Open Mod Manager"
  RMDir "$INSTDIR"

SectionEnd
