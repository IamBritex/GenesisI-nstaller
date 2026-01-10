@echo off
set "RESOURCE_OBJ="
echo     [BUILD] Verificando recursos...

REM --- LOGICA DE ICONO PLANA ---
if not exist "%PROJECT_DIR%\dist\resource.rc" goto SkipIcon
for %%R in ("%PROJECT_DIR%\dist\resource.rc") do if %%~zR LEQ 0 goto SkipIcon

echo     [RC] Compilando icono...
rc.exe /nologo /fo "%PROJECT_DIR%\dist\resource.res" "%PROJECT_DIR%\dist\resource.rc"
if errorlevel 1 (
    echo     [ERROR] Fallo al compilar el icono.
    exit /b 1
)
set "RESOURCE_OBJ="%PROJECT_DIR%\dist\resource.res""

:SkipIcon

set "INC_WEBVIEW=%CORE_DIR%\packages\Microsoft.Web.WebView2.1.0.2903.40\build\native\include"
set "LIB_WEBVIEW=%CORE_DIR%\packages\Microsoft.Web.WebView2.1.0.2903.40\build\native\x64"

echo     [CL] Ejecutando compilador C++...
echo     [TARGET] %EXE_NAME%.exe

cl.exe /nologo /EHsc /std:c++17 /D_UNICODE /DUNICODE /O2 ^
    /D "GENESIS_APP_ID=\"%APP_ID%\"" ^
    /I "%CORE_DIR%" /I "%INC_WEBVIEW%" ^
    "%CORE_DIR%\main.cpp" ^
    %RESOURCE_OBJ% ^
    /link /LIBPATH:"%LIB_WEBVIEW%" /LIBPATH:"%CORE_DIR%\discord" ^
    /DELAYLOAD:discord_game_sdk.dll ^
    delayimp.lib WebView2LoaderStatic.lib discord_game_sdk.dll.lib user32.lib gdi32.lib shell32.lib ole32.lib advapi32.lib ^
    /OUT:"%PROJECT_DIR%\dist\%EXE_NAME%.exe"

if %errorlevel% equ 0 (
    echo     [LIMPIEZA] Borrando archivos temporales...
    del "%PROJECT_DIR%\dist\*.obj" 2>nul
    del "%PROJECT_DIR%\dist\resource.rc" 2>nul
    del "%PROJECT_DIR%\dist\resource.res" 2>nul
    echo     [OK] Binario generado: dist\%EXE_NAME%.exe
) else (
    echo     [ERROR] Fallo la compilacion de C++.
    exit /b 1
)
exit /b 0