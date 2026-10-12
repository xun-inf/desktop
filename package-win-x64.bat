@echo off
setlocal
rem Build the Windows x64 installer. Pass --no-pause for terminal / CI use.
set "CODE=0"
where npm.cmd >nul 2>nul
if errorlevel 1 (
  echo [package-win-x64] npm is unavailable. Install Node.js and add it to PATH.
  set "CODE=9009"
  goto result
)

pushd "%~dp0electron"
if errorlevel 1 (
  echo [package-win-x64] Cannot enter the electron directory.
  set "CODE=1"
  goto result
)

if exist "node_modules\.bin\electron-builder.cmd" goto package
call npm.cmd ci
if errorlevel 1 (
  set "CODE=%ERRORLEVEL%"
  goto cleanup
)

:package
call npm.cmd run dist:win
set "CODE=%ERRORLEVEL%"

:cleanup
popd

:result
echo.
if "%CODE%"=="0" (
  echo [package-win-x64] OK. Installer output: %~dp0build\installer
) else (
  echo [package-win-x64] FAILED, exit code %CODE%
)
if /i not "%~1"=="--no-pause" pause
exit /b %CODE%
