!include "MUI.nsh"
!include "installer\FileAssociation.nsh"

Name "klystrack ${VERSION}"
OutFile "zip\klystrack-${VERSION}.exe"
InstallDirRegKey HKCU "Software\klystrack" ""
InstallDir $PROGRAMFILES32\klystrack
RequestExecutionLevel admin

;--------------------------------

; Pages

!insertmacro MUI_PAGE_LICENSE "LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES

; Uninst

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------

!insertmacro MUI_LANGUAGE "English"

; The stuff to install
Section "klystrack files"
SectionIn RO

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

  ; Put file there
  File zip\data\LICENSE
  File zip\data\SDL2.dll
  File zip\data\SDL.txt
  File zip\data\SDL_image.txt
  File zip\data\SDL2_image.dll
  File zip\data\zlib1.dll
  File zip\data\libpng16-16.dll
  File zip\data\Default.kt

  SetOutPath $INSTDIR\res

  File zip\data\res\*

  SetOutPath $INSTDIR\key

  File zip\data\key\FT2
  File zip\data\key\AZERTY
  File zip\data\key\DVORAK
  File zip\data\key\n00bstar

  CreateDirectory $INSTDIR\examples
  CreateDirectory $INSTDIR\examples\songs
  CreateDirectory $INSTDIR\examples\instruments

  SetOutPath $INSTDIR\examples

  File /r examples\*

  ; For CreateShortCut

  SetOutPath $INSTDIR

  File zip\data\klystrack.exe

  CreateDirectory "$SMPROGRAMS\klystrack"
  CreateShortCut "$SMPROGRAMS\klystrack\klystrack.lnk" $INSTDIR\klystrack.exe
  CreateShortCut "$SMPROGRAMS\klystrack\Uninstall klystrack.lnk" $INSTDIR\uninstall.exe

  WriteRegStr HKCU "Software\klystrack" "" $INSTDIR
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\klystrack" "DisplayName" "klystrack (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\klystrack" "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\klystrack" "UninstallString" "$INSTDIR\uninstall.exe"

  WriteUninstaller "$INSTDIR\uninstall.exe"

SectionEnd ; end the section

Section "Register file types"
  ${registerExtension} "$INSTDIR\klystrack.exe" ".kt" "klystrack song"
SectionEnd

;;;;;;;;

Section Uninstall
  ${unregisterExtension} ".kt" "klystrack song"

  Delete /REBOOTOK "$PROFILE\.klystrack"
  DeleteRegKey /ifempty HKCU "Software\klystrack"

  SetOutPath $INSTDIR

  Delete LICENSE
  Delete SDL2.dll
  Delete SDL2_image.dll
  Delete SDL.txt
  Delete SDL_image.txt
  Delete zlib1.dll
  Delete libpng16-16.dll
  Delete klystrack.exe
  Delete Default.kt

  SetOutPath $INSTDIR\key

  Delete FT2
  Delete AZERTY
  Delete DVORAK
  Delete n00bstar

  ; remove all themes

  SetOutPath $INSTDIR\res

  !tempfile deletetemp
  !system '"installer\UnFiles.cmd" "res" "${deletetemp}"'
  !include ${deletetemp}
  !delfile ${deletetemp}

  ; remove all examples

  SetOutPath $INSTDIR\examples

  !tempfile deletetemp2
  !system '"installer\UnFiles.cmd" "examples" "${deletetemp2}"'
  !include ${deletetemp2}
  !delfile ${deletetemp2}

  SetOutPath $TEMP

  Delete "$SMPROGRAMS\klystrack\klystrack.lnk"
  Delete "$SMPROGRAMS\klystrack\Uninstall klystrack.lnk"

  RMDir $SMPROGRAMS\klystrack

  RMDir $INSTDIR\examples\songs
  RMDir $INSTDIR\examples\songs\n00bstar-examples
  RMDir $INSTDIR\examples\instruments
  RMDir $INSTDIR\examples\instruments\n00bstar-instruments
  RMDir $INSTDIR\examples
  RMDir $INSTDIR\res
  RMDir $INSTDIR\key

  Delete $INSTDIR\uninstall.exe

  RMDir $INSTDIR
SectionEnd
