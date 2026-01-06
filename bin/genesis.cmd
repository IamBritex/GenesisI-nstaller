@echo off
REM Wrapper robusto para pasar argumentos
powershell -NoProfile -ExecutionPolicy Bypass -Command "& '%~dp0genesis.ps1' %*"