!include "MUI.nsh"

!define UninstLog "uninstall.log"
Var UninstLog
 
; Uninstall log file missing.
LangString UninstLogMissing ${LANG_ENGLISH} "${UninstLog} not found!$\r$\nUninstallation cannot proceed!"
 
; AddItem macro
!macro AddItem Path
 FileWrite $UninstLog "${Path}$\r$\n"
!macroend
!define AddItem "!insertmacro AddItem"
 
; File macro
!macro File FilePath FileName
 IfFileExists "$OUTDIR\${FileName}" +2
  FileWrite $UninstLog "$OUTDIR\${FileName}$\r$\n"
 File "${FilePath}${FileName}"
!macroend
!define File "!insertmacro File"
 
; CreateShortcut macro
!macro CreateShortCut FilePath FilePointer
 FileWrite $UninstLog "${FilePath}$\r$\n"
 CreateShortCut "${FilePath}" "${FilePointer}"
!macroend
!define CreateShortcut "!insertmacro CreateShortcut"

; Copy files macro
!macro CopyFiles SourcePath DestPath
 IfFileExists "${DestPath}" +2
  FileWrite $UninstLog "${DestPath}$\r$\n"
 CopyFiles "${SourcePath}" "${DestPath}"
!macroend
!define CopyFiles "!insertmacro CopyFiles"
 
; Rename macro
!macro Rename SourcePath DestPath
 IfFileExists "${DestPath}" +2
  FileWrite $UninstLog "${DestPath}$\r$\n"
 Rename "${SourcePath}" "${DestPath}"
!macroend
!define Rename "!insertmacro Rename"
 
; CreateDirectory macro
!macro CreateDirectory Path
 CreateDirectory "${Path}"
 FileWrite $UninstLog "${Path}$\r$\n"
!macroend
!define CreateDirectory "!insertmacro CreateDirectory"
 
; SetOutPath macro
!macro SetOutPath Path
 SetOutPath "${Path}"
 FileWrite $UninstLog "${Path}$\r$\n"
!macroend
!define SetOutPath "!insertmacro SetOutPath"
 
; WriteUninstaller macro
!macro WriteUninstaller Path
 WriteUninstaller "${Path}"
 FileWrite $UninstLog "${Path}$\r$\n"
!macroend
!define WriteUninstaller "!insertmacro WriteUninstaller"
 
Section -openlogfile
 CreateDirectory "$INSTDIR"
 IfFileExists "$INSTDIR\${UninstLog}" +3
  FileOpen $UninstLog "$INSTDIR\${UninstLog}" w
 Goto +4
  SetFileAttributes "$INSTDIR\${UninstLog}" NORMAL
  FileOpen $UninstLog "$INSTDIR\${UninstLog}" a
  FileSeek $UninstLog 0 END
SectionEnd

; The name of the installer
Name "klystrack ${VERSION}"

; The file to write
OutFile "zip\klystrack-${VERSION}.exe"

InstallDirRegKey HKCU "Software\klystrack" ""

; The default installation directory
InstallDir $PROGRAMFILES32\klystrack

; Request application privileges for Windows Vista
RequestExecutionLevel user

;--------------------------------

; Pages

!insertmacro MUI_PAGE_LICENSE "doc\LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

; Uninst

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------

!insertmacro MUI_LANGUAGE "English"

; The stuff to install
Section "" ;No components page, name is not important

  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR
  
  ; Put file there
  ${File} "zip\data\" klystrack.exe
  ${File} "zip\data\" LICENSE
  ${File} "zip\data\" SDL.dll
  ${File} "zip\data\" SDL.txt
  ${File} "zip\data\" SDL_mixer.dll
  
  ${SetOutPath} $INSTDIR\res
  
  ${File} "zip\data\res\" Default
  
  ${CreateDirectory} $INSTDIR\examples
  
  ${SetOutPath} $INSTDIR\examples\instruments
  
  ${File} "zip\data\examples\instruments\" bass.ki
  ${File} "zip\data\examples\instruments\" bass2.ki
  ${File} "zip\data\examples\instruments\" bigsnare.ki
  ${File} "zip\data\examples\instruments\" clap.ki
  ${File} "zip\data\examples\instruments\" cowbell.ki
  ${File} "zip\data\examples\instruments\" hardkick.ki
  ${File} "zip\data\examples\instruments\" kick.ki
  ${File} "zip\data\examples\instruments\" lead1.ki
  ${File} "zip\data\examples\instruments\" major.ki
  ${File} "zip\data\examples\instruments\" ssnare.ki
  ${File} "zip\data\examples\instruments\" stabmaj.ki
  ${File} "zip\data\examples\instruments\" stabmin.ki
  ${File} "zip\data\examples\instruments\" tom.ki
  
  ${SetOutPath} $INSTDIR\examples\songs
  
  ${File} "zip\data\examples\songs\" ringmod.kt
  
  ${CreateDirectory} "$SMPROGRAMS\klystrack"
  ${CreateShortCut} "$SMPROGRAMS\klystrack\klystrack.lnk" $INSTDIR\klystrack.exe
  ${CreateShortCut} "$SMPROGRAMS\klystrack\Uninstall klystrack.lnk" $INSTDIR\uninstall.exe
  
  WriteRegStr HKCU "Software\klystrack" "" $INSTDIR
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\klystrack" "DisplayName" "klystrack (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\klystrack" "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\klystrack" "UninstallString" "$INSTDIR\uninstall.exe"
  
  ${WriteUninstaller} "$INSTDIR\uninstall.exe"
  
SectionEnd ; end the section

;;;;;;;;

Section -closelogfile
 FileClose $UninstLog
 SetFileAttributes "$INSTDIR\${UninstLog}" READONLY|SYSTEM|HIDDEN
SectionEnd
 
Section Uninstall
 
 ; Can't uninstall if uninstall log is missing!
 IfFileExists "$INSTDIR\${UninstLog}" +3
  MessageBox MB_OK|MB_ICONSTOP "$(UninstLogMissing)"
   Abort
 
 Delete /REBOOTOK "${PROFILE}\.klystrack"
 DeleteRegKey /ifempty HKCU "Software\klystrack"
 
 Push $R0
 Push $R1
 Push $R2
 SetFileAttributes "$INSTDIR\${UninstLog}" NORMAL
 FileOpen $UninstLog "$INSTDIR\${UninstLog}" r
 StrCpy $R1 -1
 
 GetLineCount:
  ClearErrors
  FileRead $UninstLog $R0
  IntOp $R1 $R1 + 1
  StrCpy $R0 $R0 -2
  Push $R0   
  IfErrors 0 GetLineCount
 
 Pop $R0
 
 LoopRead:
  StrCmp $R1 0 LoopDone
  Pop $R0
 
  IfFileExists "$R0\*.*" 0 +3
   RMDir $R0  #is dir
  Goto +3
  IfFileExists $R0 0 +2
   Delete $R0 #is file
 
  IntOp $R1 $R1 - 1
  Goto LoopRead
 LoopDone:
 FileClose $UninstLog
 Delete "$INSTDIR\${UninstLog}"
 RMDir "$INSTDIR"
 Pop $R2
 Pop $R1
 Pop $R0
SectionEnd
