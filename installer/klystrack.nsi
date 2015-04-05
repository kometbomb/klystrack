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
  File zip\data\SDL2.dll
  File zip\data\SDL.txt
  File zip\data\SDL_image.txt
  File zip\data\SDL2_image.dll
  File zip\data\zlib1.dll
  File zip\data\libpng16-16.dll
  File zip\data\Default.kt
  
  SetOutPath $INSTDIR\res
  
  File zip\data\res\Default
  File zip\data\res\AHX
  File zip\data\res\C64
  File zip\data\res\Gameboy
  File zip\data\res\Classic
  File "zip\data\res\Golden_Brown"
  File zip\data\res\Blacklyst
  File "zip\data\res\Rust_Camo"
  File "zip\data\res\Rust_Red"
  File "zip\data\res\Ultimate_Proctamed"
  
  SetOutPath $INSTDIR\key
  
  File zip\data\key\FT2
  File zip\data\key\AZERTY
  
  CreateDirectory $INSTDIR\examples
  
  SetOutPath $INSTDIR\examples\instruments
    
  File zip\data\examples\instruments\bass.ki
  File zip\data\examples\instruments\bass2.ki
  File zip\data\examples\instruments\bigsnare.ki
  File zip\data\examples\instruments\clap.ki
  File zip\data\examples\instruments\dang.ki
  File zip\data\examples\instruments\cowbell.ki
  File zip\data\examples\instruments\hardkick.ki
  File zip\data\examples\instruments\katana.ki
  File zip\data\examples\instruments\JDsnare.ki
  File zip\data\examples\instruments\kick.ki
  File zip\data\examples\instruments\lead1.ki
  File zip\data\examples\instruments\lethal_kick.ki
  File zip\data\examples\instruments\lethal_snare.ki
  File zip\data\examples\instruments\major.ki
  File zip\data\examples\instruments\MontyKick.ki
  File zip\data\examples\instruments\snappy.ki
  File zip\data\examples\instruments\ssnare.ki
  File zip\data\examples\instruments\stabmaj.ki
  File zip\data\examples\instruments\stabmin.ki
  File zip\data\examples\instruments\the_horror.ki
  File zip\data\examples\instruments\tom.ki
  File zip\data\examples\instruments\vibbas_flt.ki
  File zip\data\examples\instruments\wetkick.ki
  
  SetOutPath $INSTDIR\examples\songs
  
  File "zip\data\examples\songs\ringmod.kt"
  File "zip\data\examples\songs\Sprock'n'Sprawl.kt"
  File "zip\data\examples\songs\phonkeh.kt"
  File "zip\data\examples\songs\obspatial.kt"
  File "zip\data\examples\songs\Paranoimia (Suntronic).kt"
  File "zip\data\examples\songs\hskv03-rygar_trance.kt"
  File "zip\data\examples\songs\castlevania.kt" 
  File "zip\data\examples\songs\dr.happy.kt"
  File "zip\data\examples\songs\hwv 437.kt" 
  File "zip\data\examples\songs\smp_dpintro.kt"
  File "zip\data\examples\songs\smp_dingleberries_fix.kt"
  File "zip\data\examples\songs\Diverted_Experience.kt"
  
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
  Delete SDL.txt
  Delete SDL2_image.txt
  Delete SDL_image.dll
  Delete zlib1.dll
  Delete libpng16-16.dll
  Delete klystrack.exe
  
  SetOutPath $INSTDIR\res
  
  Delete Default
  Delete AHX
  Delete C64
  Delete Gameboy
  Delete Classic
  Delete "Golden_Brown"
  Delete Blacklyst
  Delete "Rust_Camo"
  Delete "Rust_Red"
  Delete "Ultimate_Proctamed"
  
  SetOutPath $TEMP\key
  
  Delete FT2
  Delete AZERTY
  
  SetOutPath $INSTDIR\examples\instruments
  
  Delete bass.ki
  Delete bass2.ki
  Delete bigsnare.ki
  Delete clap.ki
  Delete dang.ki
  Delete cowbell.ki
  Delete hardkick.ki
  Delete katana.ki
  Delete JDsnare.ki
  Delete kick.ki
  Delete lead1.ki
  Delete lethal_kick.ki
  Delete lethal_snare.ki
  Delete major.ki
  Delete MontyKick.ki
  Delete snappy.ki
  Delete ssnare.ki
  Delete stabmaj.ki
  Delete stabmin.ki
  Delete the_horror.ki
  Delete tom.ki
  Delete vibbas_flt.ki
  Delete wetkick.ki
  
  SetOutPath $INSTDIR\examples\songs
  
  Delete "ringmod.kt"
  Delete "Sprock'n'Sprawl.kt"
  Delete "phonkeh.kt"
  Delete "obspatial.kt"
  Delete "Paranoimia (Suntronic).kt"
  Delete "hskv03-rygar_trance.kt"
  Delete "castlevania.kt"
  Delete "dr.happy.kt"
  Delete "hwv 437.kt" 
  Delete "smp_dpintro.kt"
  Delete "smp_dingleberries_fix.kt"
  Delete "Diverted_Experience.kt"
  
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
