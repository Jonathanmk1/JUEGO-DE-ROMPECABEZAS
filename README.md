# ROMPECABEZAS CON KINECT

## Descripción

Juego de rompecabezas controlado con Kinect y renderizado con SDL2. El objetivo es armar una imagen dividida en piezas moviendo la mano frente al sensor. El proyecto combina entrada de Kinect visualización y la lógica del rompecabezas en C.

## Estado actual (septiembre 2025)

* Compila y corre correctamente en Windows con Visual Studio 2022 (MSVC) en x64.
* Se usa Kinect SDK v1.8 y las bibliotecas SDL2 / SDL2_image (instaladas vía vcpkg).
* El ejecutable resultante es `puzzle_kinect_final.exe`.
* El build activo usa estos archivos:

  * `src/main_sdl_kinect.c` (ventana SDL y bucle del juego, integración con Kinect).
  * `puzzle_kinect_final.c` (lógica del rompecabezas en C).

## Requisitos

* Windows 10/11 (x64).
* Visual Studio 2022 (abra la terminal "x64 Native Tools Command Prompt for VS 2022").
* Kinect para Windows SDK v1.8 instalado.
* vcpkg con paquetes x64:

  * `sdl2`, `sdl2-image`.
* DLLs en tiempo de ejecución junto al `.exe` (o en PATH): `SDL2.dll`, `SDL2_image.dll`, `libpng16.dll`, `zlib1.dll`.
* Sensor Kinect v1 conectado (Kinect for Windows o Xbox 360 con adaptador) y drivers del SDK 1.8.

## Compilación (MSVC / línea de comandos)

1. Abra "x64 Native Tools Command Prompt for VS 2022".
2. Cambie a la carpeta del proyecto:

```
cd /d "C:\Users\jonas\Downloads\rompecabezas-kinect-c"
```

3. Compile con:

```
cl /EHsc /Zi /std:c++20 /TP /DNOMINMAX /DBUILD_AS_LIB ^
  /I"C:\Program Files\Microsoft SDKs\Kinect\v1.8\inc" ^
  /I"C:\vcpkg\installed\x64-windows\include" ^
  /I"C:\vcpkg\installed\x64-windows\include\SDL2" ^
  src\main_sdl_kinect.cpp ^
  game_bridge.cpp ^
  puzzle_kinect_final.c ^
  /link /MACHINE:X64 ^
  /LIBPATH:"C:\Program Files\Microsoft SDKs\Kinect\v1.8\lib\amd64" Kinect10.lib ^
  /LIBPATH:"C:\vcpkg\installed\x64-windows\lib" SDL2.lib SDL2_image.lib Ole32.lib OleAut32.lib ^
  User32.lib Gdi32.lib Shell32.lib Comdlg32.lib Shlwapi.lib ^
  /OUT:puzzle_kinect_final.exe
```

## Notas de compilación

* Ajuste las rutas de `Kinect\v1.8` y `C:\vcpkg\installed\x64-windows` si están en otra ubicación.
* `DNOMINMAX` evita conflictos con macros de Windows. `DBUILD_AS_LIB` serigrafía interna usada por la parte C.
* Si Visual Studio no encuentra SDL2/SDL2_image, verifique que estén instalados en vcpkg para x64 y que el triplete (`x64-windows`) coincide con el de su compilación.

## Ejecución

Desde la misma terminal, ejecute:

```
puzzle_kinect_final.exe
```

Coloque la(s) imagen(es) de rompecabezas en una carpeta de assets del proyecto o en la ruta que el código espera por defecto. Si desea cambiar la imagen o el tamaño del marco, modifique los valores correspondientes en el código fuente (ver `puzzle_kinect_final.c` y/o `src/main_sdl_kinect.cpp`).

## Controles (visión general)

* Seguimiento de mano con Kinect v1.
* Arrastre de piezas con la posición de la mano.
* Encaje de pieza a celda por cercanía.

## Detalles internos

* `src/main_sdl_kinect.cpp`: inicializa SDL, crea la ventana, gestiona el bucle principal, obtiene datos del Kinect y genera eventos de juego.
* `game_bridge.h/.cpp`: interfaz C++ que traduce los eventos/estados hacia/desde la lógica C del rompecabezas.
* `puzzle_kinect_final.c`: representación de tablero, piezas, barajado y verificación de estado ganado.

## Estructura del proyecto

* `src/`

  * `main_sdl_kinect.cpp`
* `game_bridge.h`
* `game_bridge.cpp`
* `puzzle_kinect_final.c`
* `assets/` (imágenes del rompecabezas) [opcional]
* `bin/` (salida sugerida para el .exe y DLLs) [opcional]


## Solución de problemas

* "No se detecta el sensor": verifique que el Kinect v1 esté encendido, que el SDK 1.8 esté instalado y que el dispositivo aparezca en el Administrador de dispositivos.
* "Faltan DLLs de SDL": coloque `SDL2.dll`, `SDL2_image.dll`, `libpng16.dll`, `zlib1.dll` junto al `.exe` o añada su ruta al `PATH`.
* "No compila por rutas": confirme las rutas de `Kinect\v1.8` y de vcpkg (triplete `x64-windows`).


## Evidencia
![menu](image.png)
![JUEGO](image-1.png)
![Ev_terminal de deteccion de movimiento](image-2.png)
