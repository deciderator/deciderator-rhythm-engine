# Pruebas en tu PC

## Funciones de v0.2.0

- Probar SPACE / FX y volver a RHYTHM / FX sin que los efectos se apaguen por
  cambiar de página. Escuchar la cola de reverb, SIZE, DECAY, AIR y SPACE MOD.
- Probar STUTTER con cada SLICE a 80/120/140 BPM, play/stop, saltos y loop.
  Se activa sobre los pasos 02/04/06/etc.; no es un granulator completo.
- Probar LO-FI, BITS y HOLD y comprobar que mezcla 0 conserva la señal previa.
- GUARDAR un .decipreset, cambiar controles, CARGAR y comprobar los 32 pasos y
  todos los efectos. Probar la selección desde la carpeta propuesta y < / >.
- Cerrar el editor con un diálogo de archivos abierto y confirmar que no hay
  crash. Cancelar guardar/cargar debe conservar el estado. Probar permisos y
  confirmación de sobrescritura en el diálogo nativo de Windows.
- Abrir CREDITOS y desplazarse por los textos de licencia; cerrar esa ventana.
- Guardar/reabrir un FLP real y renderizar colas. Medir CPU con varias instancias.

Los tests automáticos prueban lectura/escritura de presets e inválidos, no todos
los diálogos nativos ni las interacciones del host.

## Interfaz escalable de v0.1.1

En FL Studio, probar los seis tamaños del selector y arrastrar la esquina de la
ventana. Confirmar que el texto y los controles son accesibles y que arrastrar
las barras sigue actuando sobre el paso correcto. Guardar/cerrar/reabrir el FLP
y comprobar el tamaño. Repetir con escalado Windows 100/150/200 % y al mover la
ventana entre monitores. La comprobación de geometría automatizada no sustituye
estas pruebas de interacción y de negociación de tamaño con el wrapper del host.

## Funciones creativas de v0.1.0

| Prueba | Procedimiento y aceptación |
|---|---|
| Escenas | Reproducir un pad sostenido y comparar Pulse, Shards y Drift; deben producir movimiento audible distinto. Neutral debe volver a señal sin efectos con salida 0 dB. |
| Patrones | Editar ambos carriles. Verificar que Pulse actúa al subir PULSE y Tone al subir TONE. Un doble clic restablece cada barra a 1. |
| Sincronización | En FL Studio, probar 90/120/140 BPM, play/stop, seek y loop. Con STEP 1/16, los 16 pasos deben durar cuatro negras. Comparar reproducción y render offline. |
| Generación | GENERATE debe cambiar patrones/efectos y conservar salida y tempo. UNDO/REDO alterna con el estado previo a esa operación. |
| Eco | Escuchar tiempos 1/8, 1/8 con puntillo y 1/4, alternancia L/R y colas. Con FEEDBACK 0.7 comprobar que las repeticiones decaen. |
| Mezcla | DRY/WET a 0 debe conservar el original, sujeto a OUTPUT. Comparar nivel al subir Heat; no hay limitador. |
| Sesiones | Guardar y reabrir todos los pasos y parámetros. Abrir una copia de un FLP con v0.0.1: ganancia conservada y efectos nuevos neutros. |
| Actualización | Cerrar FL Studio antes de cambiar el bundle. Verificar versión 0.1.0 en el editor y reescanear si el host mantiene información anterior. |
| CPU e interfaz | Probar varias instancias, automatización simultánea, escalado Windows y buffers pequeños. La captura renderizada del editor no sustituye esta prueba interactiva. |

## Base de ganancia y host

Registrar versión exacta de Windows, FL Studio 26, driver de audio, CPU, frecuencia,
buffer y commit usado. No marcar como aprobado un punto sin ejecutarlo.

| Prueba | Procedimiento y aceptación |
|---|---|
| Compilación | Ejecutar los tres comandos del README; build y CTest deben terminar con código 0. |
| Descubrimiento | Escanear el bundle VST3 x64; FL Studio debe identificarlo como efecto y abrirlo sin errores. |
| Unidad | Con 0 dB y sin otros efectos, comparar entrada/salida o hacer null test; no debe cambiar señal. |
| Extremos | Con señal de bajo nivel, medir -60, -6 y +12 dB tras 20 ms de asentamiento; L/R conservan proporción. |
| Estéreo | Señal sólo en L, después sólo en R; no debe haber cruce entre canales. |
| Automatización | Crear Automation Clip del parámetro Gain; comprobar cambios, GUI y ausencia de clics evidentes. La automatización se aplica por bloque y se suaviza; no se promete precisión por muestra. |
| Persistencia | Ajustar -17.25 dB, guardar FLP, cerrar y reabrir FL Studio; confirmar valor y audio. Repetir con varias instancias distintas. |
| GUI | Abrir/cerrar editor repetidamente, introducir valores, doble clic a 0 dB; probar escalado Windows 100/150/200 %. |
| Audio | Probar 44.1/48/96 kHz y buffers 64/256/1024; reproducción, pausa, cambio de buffer y render offline. |
| Bypass | Probar bypass del slot y reactivación, sin crash y con nivel esperado. |
| Estabilidad | Reproducir 30 minutos y renderizar un proyecto con varias instancias; revisar consumo y errores. |
| Validación VST3 | Opcional: ejecutar pluginval o el validador de Steinberg sobre el bundle y guardar versión y log. No ejecutado aquí. |

Las pruebas C++ cubren el procesador directamente; no sustituyen la validación del
wrapper VST3, la GUI o la integración con FL Studio.

## v0.3.0 en FL Studio
Abrir TIME LAB y FX WINDOW simultáneamente; redimensionar, mover entre pantallas, cerrar y reabrir. Dibujar/automatizar la curva con audio; probar Reverse/Scratch y cambio de tempo. Guardar FLP/preset y comprobar la curva al reabrir. El buffer de audio no se restaura. Ver VERIFICATION.md para pruebas ejecutadas.
