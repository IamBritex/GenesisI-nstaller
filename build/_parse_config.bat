@echo off
if not exist "%PROJECT_DIR%\windowConfig.json" (
    echo     [ERROR] No existe windowConfig.json
    exit /b 1
)

set "PS_SCRIPT=%TEMP%\genesis_build_debug.ps1"
if exist "%PS_SCRIPT%" del "%PS_SCRIPT%"
if exist "%CONFIG_OUT%" del "%CONFIG_OUT%"

echo     [CONFIG] Generando script de analisis...

REM --- Generador PowerShell Seguro ---
echo $ErrorActionPreference = 'Stop' >> "%PS_SCRIPT%"
echo Write-Host "    [JSON] Leyendo windowConfig.json..." >> "%PS_SCRIPT%"
echo try { >> "%PS_SCRIPT%"
echo     $content = Get-Content -Raw -Path '%PROJECT_DIR%\windowConfig.json' >> "%PS_SCRIPT%"
echo     $json = $content ^| ConvertFrom-Json >> "%PS_SCRIPT%"
echo } catch { >> "%PS_SCRIPT%"
echo     Write-Host "    [ERROR] Archivo JSON invalido." >> "%PS_SCRIPT%"
echo     exit 1 >> "%PS_SCRIPT%"
echo } >> "%PS_SCRIPT%"

echo if ([string]::IsNullOrWhiteSpace($json.appID)) { Write-Host "    [ERROR] Falta appID"; exit 1 } >> "%PS_SCRIPT%"
echo $roam = Join-Path $env:APPDATA $json.appID >> "%PS_SCRIPT%"
echo $assets = Join-Path $roam 'assets' >> "%PS_SCRIPT%"
echo Write-Host "    [DATA] AppID detectado: $($json.appID)" >> "%PS_SCRIPT%"
echo if (-not (Test-Path $assets)) { New-Item -ItemType Directory -Force -Path $assets ^| Out-Null } >> "%PS_SCRIPT%"

echo Copy-Item -Path '%PROJECT_DIR%\windowConfig.json' -Destination $assets -Force >> "%PS_SCRIPT%"
echo if (Test-Path '%CORE_DIR%\discord\discord_game_sdk.dll') { Copy-Item -Path '%CORE_DIR%\discord\discord_game_sdk.dll' -Destination $roam -Force } >> "%PS_SCRIPT%"

echo if ($json.build.assets) { >> "%PS_SCRIPT%"
echo     Write-Host "    [ASSETS] Copiando archivos del proyecto..." >> "%PS_SCRIPT%"
echo     foreach($a in $json.build.assets) { >> "%PS_SCRIPT%"
echo         $s=Join-Path '%PROJECT_DIR%' $a >> "%PS_SCRIPT%"
echo         if(Test-Path $s) { Copy-Item -Path $s -Destination $assets -Recurse -Force } >> "%PS_SCRIPT%"
echo     } >> "%PS_SCRIPT%"
echo } >> "%PS_SCRIPT%"

echo $cleanTitle = if ($json.title) { $json.title -replace '[^^a-zA-Z0-9 -]', '' } else { 'GenesisApp' } >> "%PS_SCRIPT%"
echo if ([string]::IsNullOrWhiteSpace($cleanTitle)) { $cleanTitle = 'GenesisApp' } >> "%PS_SCRIPT%"

echo $rcPath = Join-Path '%PROJECT_DIR%\dist' 'resource.rc' >> "%PS_SCRIPT%"
echo $rcContent = '' >> "%PS_SCRIPT%"
echo if ($json.icon) { >> "%PS_SCRIPT%"
echo     Write-Host "    [ICONO] Procesando icono..." >> "%PS_SCRIPT%"
echo     $localIcon = Join-Path '%PROJECT_DIR%' $json.icon >> "%PS_SCRIPT%"
echo     if (Test-Path $localIcon -PathType Leaf) { >> "%PS_SCRIPT%"
echo         Copy-Item -Path $localIcon -Destination $assets -Force >> "%PS_SCRIPT%"
echo         $safeIconPath = $localIcon -replace '\\', '/' >> "%PS_SCRIPT%"
echo         $rcContent = 'IDI_ICON1 ICON "' + $safeIconPath + '"' >> "%PS_SCRIPT%"
echo     } >> "%PS_SCRIPT%"
echo } >> "%PS_SCRIPT%"
echo Set-Content -Path $rcPath -Value $rcContent -Encoding Ascii >> "%PS_SCRIPT%"

echo $outputData = "DATA_START|" + $json.appID + "|" + $cleanTitle >> "%PS_SCRIPT%"
echo $outputData ^| Out-File -FilePath '%CONFIG_OUT%' -Encoding ASCII >> "%PS_SCRIPT%"
echo Write-Host "    [OK] Configuracion lista." >> "%PS_SCRIPT%"

if not exist "%PROJECT_DIR%\dist" mkdir "%PROJECT_DIR%\dist"

powershell -NoProfile -ExecutionPolicy Bypass -File "%PS_SCRIPT%"
set "ERR=%ERRORLEVEL%"
del "%PS_SCRIPT%" 2>nul
if %ERR% NEQ 0 exit /b 1
exit /b 0