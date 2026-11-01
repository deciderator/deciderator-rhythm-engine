# Verificación de esta entrega — 2026-11-01

## Resultado final de v0.2.0

- Instalado y publicado como pre-release: https://github.com/deciderator/deciderator-rhythm-engine/releases/tag/v0.2.0
- Fuente de la versión y binario local: commit 6a2e2d78c01fa7d57e6d7599399b5ef4e61ce09c.
- GitHub Actions Windows x64, ejecución 41331411372: **SUCCESS** en configuración, compilación, CTest, pluginval nivel 5 (sin GUI en CI), empaquetado y subida del artefacto.
  https://github.com/deciderator/deciderator-rhythm-engine/actions/runs/41331411372
- ZIP público SHA256: CFA949642ABDAAFA4B49154FC81A9CB8325BA5E852B90328004A378F875DE227. Coincide con el paquete local.
- La validación local con pluginval sí incluyó los tests de editor. Se conservan logs en el paquete.
- Se abrió FL Studio 2026 para una prueba manual, pero la captura de ventana falló tras dos intentos. No se verificó la carga ni el audio dentro de FL Studio en esa prueba; se dejó abierto para el usuario.

## v0.2.0: efectos complementarios y presets

- Compilación Release x64 ejecutada con MSVC 19.52.37025, CMake 4.5.0-rc2,
  NMake y JUCE 8.0.12 fijado al mismo commit.
- CTest: 1/1 aprobado (suite completa), última ejecución registrada: 1.04 s.
  Incluye la base estéreo/ganancia, relojes, partición en bloques, estado antiguo,
  seis niveles de zoom y persistencia de dimensiones.
- Pruebas añadidas: impulso/decay/reset de SpaceFDN, periodicidad de stutter,
  cuantización y sample hold de lo-fi, roundtrip de todos los parámetros en
  .decipreset, rechazo de formato/schema/valores/duplicados inválidos, errores
  de escritura, recuperación de versiones antiguas y estrés de efectos a
  44.1/48/96 kHz con entrada fuerte y cola en silencio.
- Hubo un cierre en una ejecución intermedia con objetos compilados antes y
  después de cambiar una cabecera compartida. La reconstrucción coherente pasó.
  Se añadieron dependencias explícitas de cabeceras para evitar esa mezcla en
  MSVC/NMake localizado. La reconstrucción posterior recompiló los objetos
  propios y volvió a pasar. No se obtuvo un volcado del cierre intermedio.
- **pluginval 1.0.4, nivel 5: SUCCESS** sobre el bundle VST3 local. Se ejecutaron
  sus tests de editor, editor durante audio, procesamiento, automatización,
  estado, parámetros y buses estéreo. No se usó --skip-gui-tests en esta validación local.
  pluginval omitió el validador independiente de Steinberg porque no se configuró.
- Se conservaron capturas renderizadas de las páginas Rhythm y Space/FX al 80 %.
- Instalación local de v0.2.0 completada con FL Studio cerrado; hash del binario
  instalado idéntico al validado. Bundle anterior en outputs/backup-before-v0.2.0.
- Créditos/licencias embebidos: AGPL, dependencias y adaptación Aquaton; fuentes
  originales de referencia y licencia en third_party/aquanode.
- Se creó el repositorio público deciderator/deciderator-rhythm-engine. Estado
  de subida y ejecución CI se documenta con la entrega, sin inferir éxito por
  la mera existencia del workflow. CI usa pluginval nivel 5 sin tests GUI.

**Pendiente en el host:** escucha en FL Studio 26, reapertura de FLP reales,
diálogos nativos de guardar/cargar/cancelar/sobrescribir, cierre del editor con
diálogo abierto, DPI y CPU real con varias instancias. Los tests no certifican
calidad sonora ni rendimiento de tiempo real. Logo y rediseño visual aplazados.

Los registros de versiones anteriores siguen debajo.

## v0.1.1: ventana compacta y escalable

- Compilación local Release x64 con MSVC 19.52.37025, CMake 4.5.0-rc2 y NMake.
- CTest: 1/1 aprobado, incluyendo las comprobaciones de audio y estado previas,
  seis niveles de zoom, tamaño inicial 800 x 496, ajuste libre a 913 x 543 y
  persistencia de las dimensiones al recuperar el estado y abrir otro editor.
- La primera ejecución detectó una sobrescritura del tamaño inicial durante la
  inicialización del resizer. Se corrigió el orden y la repetición pasó completa.
- Capturas renderizadas con JUCE al 80 % y 125 % revisadas visualmente.
- Selector 75/80/90/100/125/150 %, redimensionado desde la esquina inferior derecha.
  Se mantienen proporciones; con tamaños libres pueden quedar márgenes.
- El motor y los parámetros de audio de v0.1.0 se conservan. El tamaño se guarda
  como metadatos de estado, no como un parámetro automatizable.
- Referencia visual revisada: manual de Tantra 2 suministrado por el usuario,
  páginas 5 y 9 (organización por secciones y selector de tamaño). No se copiaron
  gráficos ni código de esa interfaz.

**Pendiente en v0.1.1:** negociación de tamaño con FL Studio, ratón sobre controles
escalados, DPI de Windows, varios monitores y reapertura de FLP reales. Los tests
del editor no equivalen a probar esas interacciones dentro del host.
Reverb, glitch y lo-fi son la siguiente prioridad elegida; no se añadieron aquí.

Los apartados siguientes conservan la verificación de versiones anteriores.

## v0.1.0 creativa

- Compilación Release x64 ejecutada localmente con CMake 4.5.0-rc2, MSVC
  19.52.37025, NMake y Windows SDK 10.0.26100.0. Bundle VST3 generado.
- CTest: 1/1 aprobado. El mismo ejecutable incluye las pruebas de ganancia de
  v0.0.1 y casos nuevos: tempo/PPQ, seek, loop, transporte detenido, reloj de
  respaldo, independencia de partición en bloques, Dry/Wet transparente, gate,
  filtro, saturación, balance estéreo, eco ping-pong y borrado de cola con reset.
- Roundtrip de todos los parámetros y pasos; migración de un estado gainDb de
  v0.0.1 a efectos nuevos neutros; generación que conserva salida/tempo y deshacer.
- Prueba de señal finita/acotada con efectos activos, cambios de parámetros y
  cola en silencio a 44.1, 48 y 96 kHz, 400 bloques de 257 muestras por frecuencia.
  Esta comprobación funcional no mide rendimiento de audio en tiempo real.
- Editor renderizado desde JUCE a PNG y revisado visualmente a 1000 x 580.
  No se sustituye con esto la prueba interactiva del editor en el host.
- Se preservaron ID de fabricante/plugin, nombre e ID gainDb; el proyecto pasó
  de versión 0.0.1 a 0.1.0. Se actualizó el workflow de empaquetado.
- Instalación local de v0.1.0 completada con FL Studio cerrado, en
  C:\Program Files\Common Files\VST3\DECIDERATOR Rhythm Engine.vst3.
  Hash del binario instalado comparado con la compilación: idéntico.
  Copia del bundle anterior guardada en outputs/backup-v0.0.1.

**Pendiente:** carga, escucha, automatización, render offline, CPU, DPI y
reapertura de proyectos reales en FL Studio con **v0.1.0**. GitHub Actions y
publicación pública tampoco están verificados. El usuario mostró que v0.0.1
cargaba en FL Studio; esa captura no verifica la versión creativa nueva.

Lo siguiente conserva el registro de la entrega anterior.

## Ejecutado

- Después del reinicio se encontró Visual Studio Community 2026 Insiders 18.12,
  MSVC 19.52.37025 x64 y Windows SDK 10.0.26100.0.
- Configuración local Release con CMake 4.5.0-rc2, NMake y el entorno vcvars64:
  completada. Se reutilizó el checkout verificado de JUCE mediante
  FETCHCONTENT_SOURCE_DIR_JUCE, sin descargar otra revisión.
- Compilación de Deciderator_VST3 y DecideratorTests: completada, código de salida 0.
- CTest: **1/1 aprobado**, 0 fallos, tiempo total informado 0.13 s. La prueba cubre
  48 configuraciones de ganancia estéreo, rechazo de mono, suavizado de 20 ms,
  restauración del estado, estado inválido y bloque vacío.
- Binario VST3: cabecera PE AMD64 (0x8664) verificada. Metadatos generados por JUCE
  inspeccionados: versión 0.0.1 y clases de procesador/controlador. El manifiesto
  usa comas finales; un parseo JSON estricto lo rechazó y la inspección posterior
  las toleró sin modificar el archivo generado. Esto no es una validación VST3.
- Se corrigió el enlace de módulos JUCE en CMake para evitar compilarlos de nuevo
  innecesariamente en el wrapper VST3.
- El generador de Visual Studio 2026 falló localmente por acceso denegado en
  Microsoft.Build.Utilities.FileTracker; se usó el generador NMake con éxito.
- Inspección inicial: Git disponible; CMake no estaba disponible entonces.
- Comprobación posterior a la instalación: CMake 4.5.0-rc2 ejecutado desde
  `C:\Program Files\CMake\bin\cmake.exe`.
- Intento real de configuración con el generador `Visual Studio 17 2022`, x64
  y BUILD_TESTING=ON: falló en `project()` porque CMake no encontró ninguna
  instancia de Visual Studio. No llegó a compilar.
- Consulta remota de la etiqueta JUCE 8.0.12 y clonación de sus fuentes: revisión
  `29396c22c93392d6738e021b83196283d6e4d850`. Fijada en CMake.
- Revisión de las firmas usadas contra esas fuentes de JUCE: parámetro float,
  layout de buses, conversión Identifier/StringRef y categorías VST3 de CMake.
- Obtención del texto íntegro AGPL-3.0-only desde el catálogo SPDX:
  https://github.com/spdx/license-list-data/blob/main/text/AGPL-3.0-only.txt
- `git diff --cached --check`: sin errores de espacios en la entrega final.

## No ejecutado

- GitHub Actions: workflow preparado; sin ejecución remota verificada.
- FL Studio 26, reproducción, render offline, automatización, reapertura de FLP,
  interfaz, DPI, rendimiento y validadores VST3: requieren tu PC con host y binario.

Se incluye un `.vst3` compilado en el paquete Windows-x64. La compatibilidad con
FL Studio 26 es el objetivo de integración, todavía no un resultado de pruebas.
Consultar TESTING.md. El registro de CTest se entrega junto al binario.

## Publicación

Repositorio Git local preparado. La creación pública en GitHub no quedó verificada:
el conector reconoce la cuenta pero no tiene operación para crear repositorios;
el navegador conectado solicitó autenticación. El README incluye el procedimiento
para publicar. No se debe interpretar una URL de ejemplo como repositorio existente.
