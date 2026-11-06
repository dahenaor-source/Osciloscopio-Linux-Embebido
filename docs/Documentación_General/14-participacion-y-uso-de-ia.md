# 14. Participación y uso de inteligencia artificial

## 1. Participación del equipo

El proyecto integra firmware para ESP32-C6, comunicación SPI,
backend FastAPI en BeaglePlay, dashboard web y documentación
técnica. La distribución de funciones registrada es:

| Integrante | Área de participación |
|---|---|
| David Henao Rojas | Firmware ESP32-C6, protocolo SPI, calibración ADC, primera versión del dashboard, pruebas unitarias y documentación técnica |
| Juan Camilo Giraldo | Software de la BeaglePlay, integración del backend FastAPI, comunicación SPI y pruebas de integración |
| Laura Daniela Barragán Silva | Preparación del entorno Ubuntu y ESP-IDF; compilación y diagnóstico de programación del ESP32-C6; revisión del enlace y de las incidencias de comunicación; gestión de ramas Git; actualización y depuración del repositorio; revisión del manual, README, matriz de verificación y documentación técnica |

La atribución anterior describe la distribución documentada
y no pretende asignar autoría exclusiva sobre archivos
desarrollados de forma colaborativa.

El historial de Git conserva la trazabilidad de los cambios.


## 2. Uso de inteligencia artificial en el proyecto

### 2.1 Herramienta y alcance

- Herramienta: asistente de IA integrado en VS Code (GitHub Copilot, modo agente con acceso a terminal y archivos), usado por **David Henao Rojas** y, para las tareas descritas en la sección 2.4, por **Juan Camilo Giraldo**.
- Para **Laura Daniela Barragán Silva**, se documenta el uso de **ChatGPT** en las actividades de la sección 2.2; el modelo exacto no quedó registrado.
- Herramientas de IA declaradas por el equipo: **Copilot, Gemini y ChatGPT**. Juan Camilo Giraldo declaró haber utilizado inicialmente Gemini (Google) y posteriormente GitHub Copilot en VS Code. Las actividades de ambas herramientas se documentan por separado en las secciones 2.4 y 2.5. El modelo/versión exactos no quedaron registrados.
- La autoría Git identifica quién registró los commits, pero por sí sola no demuestra si se utilizó IA para escribir el código de esos commits. Por eso, la participación de IA de Juan Camilo se limita abajo a las tareas confirmadas en esta colaboración.

### 2.2 Tareas en las que se usó la IA (lado Laura Daniela Barragán Silva)

| Actividad | Apoyo de ChatGPT | Verificación realizada |
|---|---|---|
| Entorno Ubuntu y ESP-IDF | Orientación para configurar herramientas, ejecutar comandos y analizar errores del entorno ESP32-C6 | Revisión de las respuestas reales de la terminal y del estado de los programas instalados |
| Compilación y flasheo | Análisis de mensajes de compilación, programación y problemas de acceso a `/dev/ttyACM0` y `/dev/ttyACM1` | Comprobación de resultados en consola, diferenciando compilaciones correctas de intentos de flasheo fallidos |
| Diagnóstico e integración | Interpretación de registros SPI, inicialización del firmware, adquisición ADC y diferencias entre simulación y datos reales | Comparación con las evidencias documentadas del proyecto |
| Control de versiones | Orientación para utilizar Git, actualizar ramas, resolver divergencias y sincronizar `Osc-V2`, `Osc-V3` y `main` | Confirmación mediante `git status`, `git log`, `git diff` y resultados de `git push` |
| Depuración del repositorio | Identificación de documentos duplicados, organización de carpetas y corrección de referencias Markdown | Comprobación de los archivos conservados, enlaces internos y cambios publicados |
| Manual y README | Revisión de instrucciones de instalación, funcionamiento y estados documentados | Comparación con los procedimientos y resultados existentes |
| Matriz de verificación | Consolidación de pruebas y diferenciación entre resultados satisfactorios, históricos y parciales | Contraste con la bitácora P-01 a P-22, sin modificar los valores experimentales |
| Documentación del backend | Corrección de afirmaciones históricas sobre pérdidas SPI, 40 kSa/s y recuperación del dispositivo | Contraste con las pruebas P-18, P-19, P-20 y P-22 |
| Calibración ADC | Apoyo para interpretar las mediciones preliminares del firmware V2 | Revisión de los cuatro puntos documentados en P-21, sin atribuir nuevas mediciones |
| Declaración de participación | Organización de los aportes individuales y del uso documentado de herramientas de IA | Revisión de la coherencia entre las actividades descritas y la evidencia disponible |

### 2.3 Tareas en las que se usó la IA (lado David)

| Tarea | Qué hizo la IA | Cómo se verificó |
|---|---|---|
| Firmware ESP32-C6 (SPI esclavo, ADC continuo, protocolo, presets) | Escribió el código en C y las pruebas del protocolo | Compilación con ESP-IDF 5.4.1; 7 pruebas unitarias en el PC; flasheo y arranque en la placa real; pruebas posteriores con la BeaglePlay |
| Calibración del ADC (v2) | Propuso y programó la tabla con `adc_cali_curve_fitting` tras analizar las mediciones del equipo | Compila y pasa las pruebas unitarias; P-21 aporta una comparación preliminar hecha por Juan Camilo en CH A. La caracterización completa sigue pendiente |
| Maestro SPI de referencia (`tools/spi_master_test.py`) | Escribió el script | Se ejecutó en la BeaglePlay para P-20; el benchmark midió caudal a 1–20 MHz, pero no validó cada respuesta de las 200 transferencias |
| Dashboard HTML/JS | Escribió la interfaz y la fuente simulada | Servido por un servidor local; probado en el navegador por el equipo con datos reales. No se pasó por un analizador de sintaxis |
| Análisis de los logs de las pruebas SPI | Interpretó las salidas de `prueba_directa_spi.py` y `diagnostico_captura.py` (conteo de válidas/inválidas, patrón alternado, flags) | Los datos los tomó el equipo en la BeaglePlay; los conteos se recalcularon sobre los textos pegados |
| Driver SPI y backend (reintentos, candado, plazos) | Reescribió partes de `spi_driver.py` y `main.py` | Esclavo SPI simulado en el PC, luego pruebas del equipo en la placa real |
| Pinout y diagramas (SVG, Mermaid y gráfico) | Generó los diagramas y corrigió la numeración mikroBUS a partir de la corrección del equipo | El equipo corrigió el error de INT (11 → 15); los SVG se renderizaron para revisarlos |
| Documentación (.md) | Redactó y organizó esta documentación | Revisión por el equipo y contraste con los archivos y logs del repositorio |

### 2.4 Tareas en las que se usó la IA (lado Juan Camilo)

| Tarea | Qué hizo la IA | Cómo se verificó / participación humana |
|---|---|---|
| Revisión de resultados de pruebas de integración (P-05, P-06, P-08, P-10, P-11 y P-14) | Contrastó los resultados históricos con la evidencia posterior y propuso estados que distinguen fallas originales, mitigaciones y límites aún abiertos; actualizó la bitácora, matriz y documentos relacionados | Juan Camilo aportó el contexto y los resultados ejecutados en hardware. Se contrastaron los cambios con P-18, P-20, P-21 y P-22; las pruebas prolongadas y algunas causas raíz siguen explícitamente pendientes |
| Registro de la comparación ADC v2 (P-21) | Organizó las cuatro parejas de lecturas, calculó diferencias y porcentajes y actualizó la sección de calibración y la matriz | Juan Camilo tomó las lecturas con el multímetro y el dashboard. Los cálculos se comprobaron; la caracterización sigue parcial |
| Revalidación Nginx/API de la BeaglePlay (P-22) | Indicó comandos de comprobación, interpretó las salidas y registró la evidencia en la bitácora, la matriz y la guía de backend | Juan Camilo ejecutó los comandos en la BeaglePlay y compartió las salidas. Se verificaron HTTP 200, servicios activos/habilitados, 256 muestras por canal, `flags=0` y `retries=0`. La IA no accedió físicamente a la placa |
| Documentación de resultados y revisión de coherencia en Git | Redactó actualizaciones de la documentación de pruebas y ayudó a alinear la matriz, el README y los documentos técnicos con la evidencia disponible | Juan Camilo revisó y solicitó los ajustes; se pasó `git diff --check` y los cambios se publicaron en `main` |

El historial registra que Juan Camilo es autor de commits del backend, el driver SPI, su README y los scripts de prueba. Ese dato documenta su contribución de software, pero no basta para afirmar que la IA escribió originalmente esos componentes; la participación de IA en esa implementación debe confirmarla el integrante si desea declararla.

### 2.5 Uso previo de Gemini (Juan Camilo Giraldo)

Juan Camilo declaró haber utilizado **Gemini (Google)**
antes de migrar a **GitHub Copilot**.
La versión exacta del modelo no quedó registrada.

| Tarea | Qué hizo Gemini | Verificación reportada |
|---|---|---|
| Diagnóstico de arquitectura | Analizó latencias cercanas a 40 segundos y bloqueos al cambiar entre 100 y 40.000 Sa/s. | Pruebas de integración en ESP32-C6 y BeaglePlay. |
| Firmware ESP32-C6 | Revisó DMA dinámico, tiempos de espera del ADC y aborto de captura en `adc_capture.c` y `scope_link.c`. | Compilación y pruebas en hardware reportadas por Juan Camilo. |
| Dashboard y polling | Apoyó la adaptación del refresco a ventanas de 2,56 s a 100 Sa/s y el control de bucles simultáneos en `scope.js`. | Pruebas de interfaz y respuesta al cambiar presets. |
| Backend FastAPI | Revisó `/api/scope/frame` y `/api/scope/config` y su interacción con SPI. | Pruebas de integración del backend. |
| Control de versiones | Orientó el uso de `git reflog`, `git fetch`, `git reset --hard` y la resolución de conflictos de `Osc-V2`. También se discutió `git push -f`. | Comprobación del historial y sincronización de ramas; los comandos destructivos no se recomiendan para `main`. |
| Teoría de muestreo | Explicó Nyquist-Shannon y aliasing al adquirir señales cercanas a 2 kHz con 100 Sa/s. | Contraste teórico y experimentos con generador reportados por Juan Camilo. |

Las actividades anteriores corresponden a la declaración
de Juan Camilo. No todas las propuestas de Gemini se
consideran implementadas en V3, ni los resultados
reportados constituyen automáticamente pruebas aprobadas
en la matriz. El historial completo de Gemini no está anexado.

## 3. Verificación de resultados

- **Compilación y pruebas automáticas** donde fue posible (ESP-IDF, `make test`).
- **Pruebas en hardware reales** hechas por personas (flasheo, SPI con la BeaglePlay, multímetro).
- Para las pruebas P-18–P-22 reportadas por Juan Camilo, la ejecución física y la toma de lecturas fueron humanas; la IA ayudó a interpretar salidas compartidas y a registrar los resultados. No se deben presentar como mediciones ejecutadas autónomamente por la IA.
- **Corrección humana de errores de la IA**: el equipo detectó la numeración de pines mikroBUS errónea (INT) y aportó los logs que mostraron que `ARM` se perdía; el desajuste de la lectura del ADC se descubrió comparando con un multímetro.
- Lo **no verificado** se marca explícitamente como pendiente en [09](09-pruebas-y-resultados.md) y [10](10-matriz-de-verificacion.md).

## 4. Limitaciones de la documentación y de la IA

### Errores y límites conocidos de la IA

- Supuso un fondo de escala de 3,1 V para el ADC; la placa real no lo cumplía (ver [08](08-calibracion-adc.md)).
- Numeró mal el pin INT de mikroBUS en la primera versión.
- Los datos del AM62x (SPI maestro a 50 MHz) provienen de ejemplos de TI y **no se han confirmado** con la hoja de datos.
- Las hipótesis sobre el ruido en CH A y las transacciones perdidas son explicaciones plausibles, **no probadas**.

### Registro de interacciones con herramientas de IA

Durante el proyecto se utilizaron asistentes de inteligencia
artificial para apoyar actividades de desarrollo, diagnóstico,
análisis de resultados y documentación.

Las herramientas y actividades identificadas se describen
en las secciones 2.2 a 2.5.

La siguiente tabla presenta el estado de los historiales
de conversaciones dentro de este repositorio.

| Integrante | Herramienta documentada | Estado del historial |
|---|---|---|
| David Henao Rojas | GitHub Copilot en VS Code | No anexado. Versión exacta del modelo no documentada. |
| Juan Camilo Giraldo | Gemini (Google) y GitHub Copilot (VS Code) | No anexado. Versiones exactas de los modelos no documentadas. |
| Laura Daniela Barragán Silva | ChatGPT | No anexado. Versión exacta del modelo no documentada. |

La documentación registra las tareas apoyadas por IA y
los procedimientos de comprobación disponibles.

Sin embargo, las transcripciones completas de las
conversaciones no forman parte de los archivos publicados.

Por tanto, esta declaración documenta el uso de IA,
pero no sustituye los historiales completos cuando
estos sean exigidos como evidencia independiente.

### Criterios de verificación y responsabilidad

Las respuestas de los asistentes se consideraron
propuestas sujetas a revisión humana.

Las comprobaciones del firmware, del enlace SPI,
del backend, del dashboard y de los servicios de
la BeaglePlay se registran en la bitácora de pruebas
y en la matriz de verificación.

Los resultados experimentales pertenecen a las
pruebas realizadas por los integrantes del equipo.
No se atribuye a la IA la ejecución física de las
mediciones ni se presentan resultados parciales
como certificaciones completas.

La autoría y participación individual se describen
según las actividades documentadas, sin atribuir
automáticamente el trabajo de un integrante a otro.
