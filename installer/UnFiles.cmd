@rem NSIS Uninstall Header Recursive File List Maker
@rem Copyright 2014 Aleksandr Ivankiv

@rem Modified for the klystrack installer by Tero Lindeman

@SET DIR=%~1
@SET HEADER=%~2
@IF "%~1" == "/?" goto Help
@IF NOT DEFINED DIR goto Help
@IF NOT DEFINED HEADER SET HEADER=UnFiles.nsh
@IF NOT EXIST "%DIR%" ECHO Cannot find the folder %DIR%. & SET "DIR=" & goto :EOF

@SetLocal EnableDelayedExpansion

@FOR /F "tokens=*" %%f IN ('DIR %DIR%\*.* /A:-D /B /S') DO @(
  set string=%%f
  set string=!string:%CD%\%DIR%=!
  echo Delete "$OUTDIR\!string:~1!" >> %HEADER%
)

@FOR /F "tokens=*" %%d IN ('DIR %DIR%\*.* /A:D /B /S') DO @(
  set string=%%d
  set string=!string:%CD%\%DIR%=!
  echo RMDir "$OUTDIR\!string:~1!" >> %HEADER%
)

@EndLocal
@goto :EOF

:Help
@echo.
@echo Usage: UNFILES FolderName [OutFile]
@echo.
@goto :EOF