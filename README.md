Proyecto: Rompecabezas Controlado con Kinect v1

Este proyecto es un prototipo de videojuego en C donde el jugador resuelve un rompecabezas deslizante (tipo 8-puzzle o 15-puzzle) controlando las piezas con movimientos de la mano detectados por el sensor Kinect v1.

🚀 Objetivo del Proyecto

Desarrollar un sistema de interacción natural donde el usuario no use teclado ni mouse, sino gestos captados por Kinect.

Utilizar primero una versión en consola con números como prototipo para validar la lógica del juego y la detección de gestos.

Evolucionar después hacia un rompecabezas con imágenes reales y una interfaz gráfica completa (ventana con piezas de imagen en lugar de números).

📌 Funcionalidades actuales

Rompecabezas en consola (3×3 o 4×4):

Representado con números y un espacio vacío (.).

El objetivo es ordenar las piezas hasta completar la secuencia correcta.

Control por teclado (modo debug):

W = arriba

S = abajo

A = izquierda

D = derecha

M = mezclar el tablero

P = pausa / ready

Q = salir

Control por gestos con Kinect:

Swipe izquierda → mover la pieza hacia la izquierda.

Swipe derecha → mover la pieza hacia la derecha.

Swipe arriba → mover la pieza hacia arriba.

Swipe abajo → mover la pieza hacia abajo.

Modo prototipo lento (debug):

El tablero solo se actualiza cuando detecta un gesto válido.

Tras cada movimiento, el juego se detiene 0.7 segundos para que el jugador vea el tablero y piense el próximo movimiento.

Esto permite probar y comprender mejor la interacción, antes de pasar al modo rápido tipo videojuego.

🛠️ Tecnologías utilizadas

Lenguaje: C (compilado con MSVC / MinGW).

SDK: Microsoft Kinect SDK v1.8 (Runtime + Toolkit).

IDE / Herramientas: Visual Studio 2022 (Developer Command Prompt) y/o Visual Studio Code con MinGW.

Librerías propias:

kinect_input.c → gestión de datos del Kinect.

gesture.c → detección e interpretación de gestos.

puzzle.c → lógica del rompecabezas.

renderer.c → renderizado básico en consola (stub).

session.c → manejo de estado de la partida.

config.c → parámetros de configuración (mano dominante, sensibilidad, etc.).

🔧 Proceso de instalación y compilación

Instalar Visual Studio 2022 Community con la carga de trabajo Desktop development with C++.

Instalar Kinect SDK 1.8 (incluye runtime y headers en C:\Program Files\Microsoft SDKs\Kinect\v1.8).

Abrir el x64 Native Tools Command Prompt de Visual Studio.

Ir a la carpeta del proyecto:

cd %USERPROFILE%\Downloads\rompecabezas-kinect-c


Configurar la variable de entorno temporal:

set "SDKKINECT=C:\Program Files\Microsoft SDKs\Kinect\v1.8"


Compilar con:

cl /EHsc /Zi /std:c++20 /TP src\*.c ^
   /I"%SDKKINECT%\inc" ^
   /link /MACHINE:X64 /LIBPATH:"%SDKKINECT%\lib\amd64" Kinect10.lib ^
   /OUT:rompecabezas.exe

📋 Próximos pasos

Agregar mensajes claros de gestos en consola, por ejemplo:

"Gesto detectado: IZQUIERDA"

"Gesto detectado: ARRIBA"

Migrar a una interfaz gráfica (SDL2 u OpenGL):

Mostrar las piezas como partes de una imagen cargada por el usuario.

Dibujar un cursor de la mano o flechas de feedback visual.

Agregar HUD con contador de movimientos, tiempo, etc.

Optimización del modo de juego:

Cambiar del modo lento por turnos al modo fluido en tiempo real.

Ajustar la sensibilidad de gestos para que sean cómodos en sesión real.

👥 Estado actual del prototipo

✅ Kinect detecta manos y gestos.

✅ Los gestos ya se traducen en movimientos válidos del rompecabezas.

✅ El tablero se queda quieto tras cada jugada (modo paso a paso).

🔜 Faltan interfaz gráfica e integración con imágenes.