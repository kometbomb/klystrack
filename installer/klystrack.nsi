!include "MUI.nsh"
!include "installer\FileAssociation.nsh"

Name "klystrack ${VERSION}"
OutFile "zip\klystrack-${VERSION}.exe"
InstallDirRegKey HKCU "Software\klystrack" ""
InstallDir $PROGRAMFILES32\klystrack
RequestExecutionLevel admin

;--------------------------------

; Pages

!insertmacro MUI_PAGE_LICENSE "doc\LICENSE"
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
  File zip\data\SDL.dll
  File zip\data\SDL.txt
  File zip\data\SDL_mixer.dll
  
  SetOutPath $INSTDIR\res
  
  File zip\data\res\Default
  
  CreateDirectory $INSTDIR\examples
  
  SetOutPath $INSTDIR\examples\instruments
    
  File zip\data\examples\instruments\bass.ki
  File zip\data\examples\instruments\bass2.ki
  File zip\data\examples\instruments\bigsnare.ki
  File zip\data\examples\instruments\clap.ki
  File zip\data\examples\instruments\cowbell.ki
  File zip\data\examples\instruments\hardkick.ki
  File zip\data\examples\instruments\kick.ki
  File zip\data\examples\instruments\lead1.ki
  File zip\data\examples\instruments\major.ki
  File zip\data\examples\instruments\ssnare.ki
  File zip\data\examples\instruments\stabmaj.ki
  File zip\data\examples\instruments\stabmin.ki
  File zip\data\examples\instruments\tom.ki
  
  SetOutPath $INSTDIR\examples\songs
  
  File zip\data\examples\songs\ringmod.kt
  
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
  ${registerExtension} "$INSTDIR\klystrack.exe" ".kt" "klystracker song"
SectionEnd

;;;;;;;;

Section Uninstall
  ${unregisterExtension} ".kt" "klystracker song"

  Delete /REBOOTOK "$PROFILE\.klystrack"
  DeleteRegKey /ifempty HKCU "Software\klystrack"
 
  SetOutPath $INSTDIR
 
  Delete LICENSE
  Delete SDL.dll
  Delete SDL.txt
  Delete SDL_mixer.dll
  Delete klystrack.exe
  
  SetOutPath $INSTDIR\res
  
  Delete Default
  
  SetOutPath $INSTDIR\examples\instruments
  
  Delete bass.ki
  Delete bass2.ki
  Delete bigsnare.ki
  Delete clap.ki
  Delete cowbell.ki
  Delete hardkick.ki
  Delete kick.ki
  Delete lead1.ki
  Delete major.ki
  Delete ssnare.ki
  Delete stabmaj.ki
  Delete stabmin.ki
  Delete tom.ki
  
  SetOutPath $INSTDIR\examples\songs
  
  Delete ringmod.kt
  
  SetOutPath $TEMP
  
  Delete "$SMPROGRAMS\klystrack\klystrack.lnk" 
  Delete "$SMPROGRAMS\klystrack\Uninstall klystrack.lnk" 
  
  RMDir $SMPROGRAMS\klystrack
  
  RMDir $INSTDIR\examples\songs
  RMDir $INSTDIR\examples\instruments
  RMDir $INSTDIR\examples
  RMDir $INSTDIR\res
  
  Delete $INSTDIR\uninstall.exe
  
  RMDir $INSTDIR
SectionEnd
