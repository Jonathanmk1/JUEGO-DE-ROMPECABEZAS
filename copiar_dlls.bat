@echo off
REM Script para copiar todas las DLL necesarias desde vcpkg al proyecto

echo === Copiando DLLs desde vcpkg... ===

REM Carpeta de origen (binarios de vcpkg)
set SRC=C:\vcpkg\installed\x64-windows\bin

REM Carpeta destino (raíz del proyecto)
set DEST=C:\Users\jonas\Downloads\rompecabezas-kinect-c

REM Copiar todas las DLLs de SDL2 y dependencias (PNG, JPEG, Zlib, etc.)
copy "%SRC%\SDL2.dll" "%DEST%" >nul
copy "%SRC%\SDL2_image.dll" "%DEST%" >nul
copy "%SRC%\libpng16.dll" "%DEST%" >nul
copy "%SRC%\zlib1.dll" "%DEST%" >nul
copy "%SRC%\jpeg.dll" "%DEST%" >nul
copy "%SRC%\turbojpeg.dll" "%DEST%" >nul

echo === Copia completada ===
pause
