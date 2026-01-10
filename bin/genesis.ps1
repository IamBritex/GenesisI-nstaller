$projectDir = Get-Location
$builderPath = "C:\Genesis\build\builder.bat"

if ($args[0] -eq "compile") {
    if (Test-Path $builderPath) {
        & cmd.exe /c "`"$builderPath`" `"$projectDir`""
    } else {
        Write-Host "[ERROR] No se encuentra C:\Genesis\build\builder.bat" -ForegroundColor Red
    }
} else {
    Write-Host "Comando desconocido."
}