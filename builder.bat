@echo off
setlocal enabledelayedexpansion

set "PROJECT_DIR=%~1"
set "PROJECT_DIR=%PROJECT_DIR:"=%"
set "GENESIS_ROOT=%~dp0"
set "CORE_DIR=%GENESIS_ROOT%core"
set "OUT_DIR=%PROJECT_DIR%\dist"
set "OBJ_DIR=%PROJECT_DIR%\obj"

where cl.exe >nul 2>nul
if %ERRORLEVEL% EQU 0 goto :EntornoListo

echo  [*] Buscando compilador...
set "VS_PATH_1=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
set "VS_PATH_2=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
set "VS_PATH_3=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"

if exist "%VS_PATH_1%" ( call "%VS_PATH_1%" >nul & goto :EntornoListo )
if exist "%VS_PATH_2%" ( call "%VS_PATH_2%" >nul & goto :EntornoListo )
if exist "%VS_PATH_3%" ( call "%VS_PATH_3%" >nul & goto :EntornoListo )
exit /b 1

:EntornoListo
if not exist "%OBJ_DIR%" mkdir "%OBJ_DIR%"
if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

echo  [*] Leyendo configuracion...
powershell -NoProfile -Command ^
    "$jsonPath = Join-Path '%PROJECT_DIR%' 'windowConfig.json'; "^
    "if (-not (Test-Path $jsonPath)) { exit 1 }; "^
    "$json = Get-Content $jsonPath -Raw | ConvertFrom-Json; "^
    "$appName = $json.title; "^
    "if ([string]::IsNullOrWhiteSpace($appName)) { $appName = 'GenesisApp' }; "^
    "$safeName = $appName -replace '[^a-zA-Z0-9 \-_]', ''; "^
    "$safeName = $safeName.Trim(); "^
    "$appID = $json.appID; "^
    "$iconRel = $json.icon; "^
    "Set-Content '%OBJ_DIR%\name.tmp' $safeName; "^
    "Set-Content '%OBJ_DIR%\appid.tmp' $appID; "^
    "Set-Content '%OBJ_DIR%\icon.tmp' $iconRel; "^
    "$headerContent = '#pragma once' + [Environment]::NewLine + '#define GAME_APP_ID L\"' + $appID + '\"'; "^
    "Set-Content '%OBJ_DIR%\AppID.h' $headerContent;"

set /p APP_NAME=<"%OBJ_DIR%\name.tmp"
set /p GAME_APP_ID=<"%OBJ_DIR%\appid.tmp"

echo  [*] Copiando assets...
powershell -NoProfile -Command ^
    "$json = Get-Content '%PROJECT_DIR%\windowConfig.json' -Raw | ConvertFrom-Json; "^
    "$files = $json.build.assets; "^
    "foreach ($item in $files) { "^
    "  $src = Join-Path '%PROJECT_DIR%' $item; "^
    "  if (Test-Path $src) { Copy-Item -Path $src -Destination '%APPDATA%\%GAME_APP_ID%\assets' -Recurse -Force; } "^
    "}"

echo  [*] Generando recursos...
powershell -NoProfile -Command ^
    "$iconRel = Get-Content '%OBJ_DIR%\icon.tmp'; "^
    "$fullIcon = Join-Path '%PROJECT_DIR%' $iconRel; "^
    "$defaultIcon = Join-Path '%CORE_DIR%' 'nativeWindow\default.ico'; "^
    "$rcPath = Join-Path '%OBJ_DIR%' 'resource.rc'; "^
    "$finalIcon = if (Test-Path $fullIcon) { $fullIcon.Replace('\', '\\') } else { $defaultIcon.Replace('\', '\\') }; "^
    "$content = 'IDI_ICON1 ICON \"' + $finalIcon + '\"' + [Environment]::NewLine + 'DISCORD_DLL RCDATA \"' + ('%CORE_DIR%\discord\discord_game_sdk.dll').Replace('\', '\\') + '\"'; "^
    "Set-Content -Path $rcPath -Value $content -Encoding utf8;"

rc.exe /nologo /fo "%OBJ_DIR%\resource.res" "%OBJ_DIR%\resource.rc"

echo  [*] Compilando ejecutable...
set "WV2_VER=1.0.2903.40"
set "PKG_BASE=%CORE_DIR%\packages\Microsoft.Web.WebView2.%WV2_VER%\build\native"
set "INC_PATH=%PKG_BASE%\include"
set "LIB_PATH=%PKG_BASE%\x64"
set "CL_FLAGS=/nologo /EHsc /std:c++17 /D_UNICODE /DUNICODE /O2"
set "LINK_LIBS=delayimp.lib WebView2LoaderStatic.lib user32.lib gdi32.lib shlwapi.lib shell32.lib ole32.lib comdlg32.lib psapi.lib advapi32.lib crypt32.lib version.lib"

cl.exe %CL_FLAGS% /I "%INC_PATH%" /I "%CORE_DIR%" /I "%OBJ_DIR%" ^
    /Fo"%OBJ_DIR%\\" /Fe"%OUT_DIR%\%APP_NAME%.exe" "%CORE_DIR%\main.cpp" "%OBJ_DIR%\resource.res" ^
    "%CORE_DIR%\discord\discord_game_sdk.dll.lib" ^
    /link /DELAYLOAD:discord_game_sdk.dll /LIBPATH:"%LIB_PATH%" %LINK_LIBS%

REM Inyectar manifiesto de administrador
if exist "%PROJECT_DIR%\app.manifest" (
    echo  [*] Inyectando permisos de administrador...
    mt.exe -nologo -manifest "%PROJECT_DIR%\app.manifest" -outputresource:"%OUT_DIR%\%APP_NAME%.exe;#1"
)

echo.
echo  [OK] %OUT_DIR%\%APP_NAME%.exe
exit /b 0