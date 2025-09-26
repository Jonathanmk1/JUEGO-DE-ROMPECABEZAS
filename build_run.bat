@echo off
REM === Script para compilar, copiar DLLs y ejecutar el rompecabezas ===

REM Carpeta base del proyecto
set PROJ=C:\Users\jonas\Downloads\rompecabezas-kinect-c

REM SDK de Kinect
set SDKKINECT=C:\Program Files\Microsoft SDKs\Kinect\v1.8

REM vcpkg root
set VCPKG=C:\vcpkg\installed\x64-windows

echo === Compilando proyecto ===
cl /EHsc /Zi /std:c++20 /TP ^
  /I"%SDKKINECT%\inc" ^
  /I"%VCPKG%\include" ^
  "%PROJ%\src\main_sdl_kinect.cpp" ^
  /link /MACHINE:X64 ^
  /LIBPATH:"%SDKKINECT%\lib\amd64" Kinect10.lib ^
  /LIBPATH:"%VCPKG%\lib" SDL2.lib SDL2_image.lib Ole32.lib OleAut32.lib ^
  /OUT:"%PROJ%\rompecabezas_kinect.exe"

if errorlevel 1 (
    echo ❌ Error en la compilación
    pause
    exit /b
)

echo === Copiando DLLs necesarias ===
copy "%VCPKG%\bin\SDL2.dll" "%PROJ%" >nul
copy "%VCPKG%\bin\SDL2_image.dll" "%PROJ%" >nul
copy "%VCPKG%\bin\libpng16.dll" "%PROJ%" >nul
copy "%VCPKG%\bin\zlib1.dll" "%PROJ%" >nul
copy "%VCPKG%\bin\jpeg.dll" "%PROJ%" >nul
copy "%VCPKG%\bin\turbojpeg.dll" "%PROJ%" >nul

REM === Preguntar al usuario qué imagen usar ===
set /p IMG="Escribe la ruta de la imagen (por ejemplo: assets\LogoUAEMex.png): "

REM Si el usuario no escribe nada, usar una por defecto
if "%IMG%"=="" set IMG=assets\LogoUAEMex.png

echo === Ejecutando juego con %IMG% ===
cd /d "%PROJ%"
rompecabezas_kinect.exe --image "%IMG%" --grid 3 --size 800

pause
