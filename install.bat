@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"

set "INSTALL_DIR=C:\Genesis"
if exist "%INSTALL_DIR%" rmdir /s /q "%INSTALL_DIR%"
mkdir "%INSTALL_DIR%"

echo [*] Instalando nucleo...
xcopy /E /I /Y "core" "%INSTALL_DIR%\core" >nul
echo [*] Instalando binarios...
xcopy /E /I /Y "bin" "%INSTALL_DIR%\bin" >nul
echo [*] Instalando constructor...
xcopy /E /I /Y "build" "%INSTALL_DIR%\build" >nul

setx PATH "%PATH%;%INSTALL_DIR%\bin" /M
echo [OK] Genesis Engine instalado.
pause