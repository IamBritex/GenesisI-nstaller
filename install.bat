@echo off
setlocal enabledelayedexpansion
cls

REM =========================================================
REM  FIX: CAMBIAR AL DIRECTORIO DONDE ESTA ESTE ARCHIVO
REM =========================================================
cd /d "%~dp0"

echo  =========================================
echo    INSTALADOR DE GENESIS ENGINE CLI
echo  =========================================
echo.
echo  [DEBUG] Directorio actual: %CD%
echo.

REM 1. Verificar permisos de admin
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo [!] ERROR: Haz clic derecho y selecciona "Ejecutar como administrador".
    pause
    exit
)

REM 2. Verificar que los archivos existen ANTES de intentar copiar
if not exist "core" (
    echo [X] ERROR CRITICO: No encuentro la carpeta 'core'.
    echo     Asegurate de que la carpeta 'core' este al lado de 'install.bat'.
    pause
    exit
)
if not exist "bin" (
    echo [X] ERROR CRITICO: No encuentro la carpeta 'bin'.
    pause
    exit
)

REM 3. Preparar C:\Genesis
set "INSTALL_DIR=C:\Genesis"
if exist "%INSTALL_DIR%" (
    echo [*] Limpiando instalacion anterior...
    rmdir /s /q "%INSTALL_DIR%"
)
mkdir "%INSTALL_DIR%"

REM 4. Copiar archivos
echo [*] Copiando archivos del motor...
xcopy /E /I /Y "core" "%INSTALL_DIR%\core" >nul
if %errorLevel% neq 0 echo [X] Error copiando core.

xcopy /E /I /Y "bin" "%INSTALL_DIR%\bin" >nul
if %errorLevel% neq 0 echo [X] Error copiando bin.

copy /Y "builder.bat" "%INSTALL_DIR%\" >nul

REM 5. Agregar al PATH
echo [*] Configurando Variables de Entorno (PATH)...
set "BIN_PATH=%INSTALL_DIR%\bin"

REM Solo agrega al PATH si no esta ya (evita duplicados y errores)
echo %PATH% | find /i "%BIN_PATH%" >nul
if %errorlevel% neq 0 (
    setx PATH "%PATH%;%BIN_PATH%" /M
    echo    [+] PATH actualizado.
) else (
    echo    [=] El PATH ya estaba configurado.
)

echo.
echo  =========================================
echo    INSTALACION COMPLETADA EXITOSAMENTE
echo  =========================================
echo.
echo  1. Cierra TODAS las ventanas de terminal abiertas.
echo  2. Abre una nueva.
echo  3. Prueba escribir: genesis help
echo.
pause