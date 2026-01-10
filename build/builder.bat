@echo off
setlocal enabledelayedexpansion

REM ==================================================
REM      GENESIS ENGINE BUILD SYSTEM (FAST MODE)
REM ==================================================

REM --- [TIMER] Inicio ---
set "TIMER_XML=%TEMP%\genesis_timer_%RANDOM%.xml"
powershell -Command "Get-Date | Export-Clixml -Path '%TIMER_XML%'"

set "PROJECT_DIR=%~1"
set "PROJECT_DIR=%PROJECT_DIR:"=%"
set "BUILD_DIR=%~dp0"
set "GENESIS_ROOT=%BUILD_DIR%.."
set "CORE_DIR=%GENESIS_ROOT%\core"

echo.
echo ==================================================
echo         GENESIS ENGINE BUILD SYSTEM
echo ==================================================
echo.

echo [PASO 1/3] Inicializando entorno de Visual Studio...
call "%BUILD_DIR%_init_vs.bat" || ( 
    echo [FATAL] No se pudo iniciar el compilador. 
    call :CalculateTime
    exit /b 1 
)

echo.
echo [PASO 2/3] Procesando configuracion y assets...
set "CONFIG_OUT=%TEMP%\genesis_config_out.txt"
call "%BUILD_DIR%_parse_config.bat" || ( 
    echo [FATAL] Error en la configuracion del proyecto.
    call :CalculateTime
    exit /b 1 
)

REM Leer resultados
if exist "%CONFIG_OUT%" (
    for /f "usebackq tokens=2,3 delims=|" %%a in ("%CONFIG_OUT%") do (
        set "APP_ID=%%a"
        set "EXE_NAME=%%b"
    )
)
del "%CONFIG_OUT%" 2>nul

if "%APP_ID%"=="" (
    echo [ERROR] No se pudo obtener el AppID.
    call :CalculateTime
    exit /b 1
)

echo.
echo [PASO 3/3] Compilando binario...
call "%BUILD_DIR%_compile.bat" || ( 
    echo [FATAL] Error en la compilacion.
    call :CalculateTime
    exit /b 1 
)

echo.
echo ==================================================
echo [EXITO] Build finalizado correctamente.
echo ==================================================

call :CalculateTime
exit /b 0

:CalculateTime
    echo.
    if exist "%TIMER_XML%" (
        powershell -Command "$start=Import-Clixml -Path '%TIMER_XML%'; $diff=(Get-Date)-$start; Write-Host ('[TIEMPO] Duracion Total: {0:mm}m {0:ss}s {0:fff}ms' -f $diff)"
        del "%TIMER_XML%" 2>nul
    )
    goto :eof