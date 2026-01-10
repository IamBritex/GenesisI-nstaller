@echo off
echo     [VS] Verificando cl.exe en el PATH...
where cl.exe >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo     [OK] Compilador ya activo.
    exit /b 0
)

set "VS_1=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
set "VS_2=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

echo     [VS] Buscando instalacion de Visual Studio 2022...

if exist "%VS_1%" (
    echo     [OK] Encontrado: VS Community 2022. Inicializando...
    call "%VS_1%" >nul
    exit /b 0
)
if exist "%VS_2%" (
    echo     [OK] Encontrado: VS BuildTools 2022. Inicializando...
    call "%VS_2%" >nul
    exit /b 0
)

echo     [ERROR] No se encontro Visual Studio 2022.
exit /b 1