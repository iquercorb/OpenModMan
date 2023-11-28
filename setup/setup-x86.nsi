; Open Mod Manager NSIS script for x86 version

;--------------------------------

!include MUI2.nsh
!include OpenModMan.nsh

!define  AAP_ARCH         "x86"


InstallDir                "$PROGRAMFILES32\${APP_NAME}"
Name                      "${APP_NAME} ${AAP_MAJ}.${AAP_MIN}.${AAP_REV}"
OutFile                   "${APP_SHORT_NAME}_${AAP_MAJ}-${AAP_MIN}-${AAP_REV}-${AAP_ARCH}_setup.exe"
RequestExecutionLevel     admin
LicenseData               "..\LICENSE"

; Registry key to check for directory (so if you install again, it will
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\${APP_REG_NAME}" "Install_Dir"

; Sets the text that is shown at the bottom of the install window
BrandingText " "

; Sets the background color of the license data.
LicenseBkColor /windows

; Use modern Windows theme
XPStyle on

ShowInstDetails show
ShowUnInstDetails show

!define MUI_FINISHPAGE_RUN "$INSTDIR\${APP_EXE_NAME}.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Start ${APP_NAME}"
!define MUI_WELCOMEFINISHPAGE_BITMAP "..\res\images\setup_splash.bmp"

;--------------------------------

; Pages
!insertmacro MUI_PAGE_LICENSE "..\LICENSE"
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
Section "Install (required)"

  SectionIn RO

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

  ; Put file there
  File "/oname=${APP_EXE_NAME}.exe"       "${REL_32_EXE}"

  File "/oname=libcurl.dll"               "${CURL_32_DLL}"
  File "/oname=liblzma.dll"               "${LZMA_32_DLL}"
  File "/oname=libzstd.dll"               "${ZSTD_32_DLL}"
  File "/oname=zlib1.dll"                 "${ZLIB_32_DLL}"

  file "/oname=omm_icon.ico"              "${OMM_ICO}"
  file "/oname=omx_icon.ico"              "${OMX_ICO}"
  file "/oname=ozp_icon.ico"              "${OZP_ICO}"
  file "/oname=ozb_icon.ico"              "${OZB_ICO}"

  File "/oname=LICENSE"               "..\LICENSE"
  File "/oname=CREDITS"               "..\CREDITS"

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\${APP_REG_NAME} "Install_Dir" "$INSTDIR"

  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_REG_NAME}" "DisplayName" "${APP_NAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_REG_NAME}" "DisplayIcon" "$INSTDIR\${APP_EXE_NAME}.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_REG_NAME}" "DisplayVersion" "${AAP_MAJ}.${AAP_MIN}.${AAP_REV} (${AAP_ARCH})"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_REG_NAME}" "Publisher" "${APP_PUBLISHER}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_REG_NAME}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_REG_NAME}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_REG_NAME}" "NoRepair" 1
  WriteUninstaller "$INSTDIR\uninstall.exe"

  ; file Association
  WriteRegStr HKCR ".ozp" "" "${APP_REG_NAME}.ModPack"
  WriteRegStr HKCR ".ozp" "Content Type" "application/x-zip-compressed"
  WriteRegStr HKCR "${APP_REG_NAME}.OpenModPack\DefaultIcon" "" "$INSTDIR\ozp_icon.ico"
  WriteRegStr HKCR "${APP_REG_NAME}.OpenModPack" "" "${APP_NAME} Open Mod Pack"

  WriteRegStr HKCR ".ozb" "" "${APP_REG_NAME}.ModBack"
  WriteRegStr HKCR ".ozb" "Content Type" "application/x-zip-compressed"
  WriteRegStr HKCR "${APP_REG_NAME}.OpenModBack\DefaultIcon" "" "$INSTDIR\ozb_icon.ico"
  WriteRegStr HKCR "${APP_REG_NAME}.OpenModBack" "" "${APP_NAME} Open Mod Backup"

  WriteRegStr HKCR ".omx" "" "${APP_REG_NAME}.OpenModXml"
  WriteRegStr HKCR ".omx" "Content Type" "application/xml"
  WriteRegStr HKCR "${APP_REG_NAME}.OpenModXml\DefaultIcon" "" "$INSTDIR\omx_icon.ico"
  WriteRegStr HKCR "${APP_REG_NAME}.OpenModXml" "" "${APP_NAME} Open Mod XML definition"

SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\${APP_NAME}"
  CreateShortcut "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortcut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE_NAME}.exe" "" "$INSTDIR\${APP_EXE_NAME}.exe" 0

SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"

  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_REG_NAME}"
  DeleteRegKey HKLM SOFTWARE\${APP_REG_NAME}

  ; Remove files and uninstaller
  Delete $INSTDIR\*.*

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\${APP_NAME}\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\${APP_NAME}"
  RMDir "$INSTDIR"

  ; Remove file association
  DeleteRegKey HKCR ".ozp"
  DeleteRegKey HKCR "${APP_REG_NAME}.OpenModPack"

  DeleteRegKey HKCR ".ozb"
  DeleteRegKey HKCR "${APP_REG_NAME}.OpenModBack"

  DeleteRegKey HKCR ".omx"
  DeleteRegKey HKCR "${APP_REG_NAME}.OpenModXml"

SectionEnd
