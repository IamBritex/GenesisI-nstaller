$inputCmd = $args[0]

$version = "2.3 (Stable)"
$genesisRoot = Split-Path -Parent $PSScriptRoot
$builderPath = Join-Path $genesisRoot "builder.bat"

if ([string]::IsNullOrWhiteSpace($inputCmd)) {
    $inputCmd = "help"
}

if ($inputCmd -eq "-v" -or $inputCmd -eq "--version") {
    Write-Host "Genesis Engine CLI v$version" -ForegroundColor Cyan
}
elseif ($inputCmd -eq "help" -or $inputCmd -eq "-h") {
    Write-Host "Genesis Engine CLI v$version" -ForegroundColor Green
    Write-Host "----------------------------"
    Write-Host "  genesis compile       -> Compila el proyecto actual"
    Write-Host "  genesis -v            -> Muestra la version"
}
elseif ($inputCmd -eq "compile") {
    $currentDir = Get-Location
    if (-not (Test-Path "$currentDir\windowConfig.json")) {
        Write-Host " [X] Error: No hay 'windowConfig.json' aqui." -ForegroundColor Red
        return
    }
    & cmd.exe /c "$builderPath" "$currentDir"
}
else {
    Write-Host "Comando '$inputCmd' no reconocido." -ForegroundColor Yellow
}