# 14. Participación y uso de IA

> Este documento es una **plantilla pre-llenada con lo que consta en el repositorio y en las conversaciones**. Cada integrante debe revisar, corregir y completar su parte antes de la entrega. Lo que no se pudo confirmar está marcado como _por completar_.

## 1. Declaración de participación y aportes

Equipo (máximo 6 personas según el enunciado).

| Integrante | Rol | Actividades realizadas (según el repositorio) |
|---|---|---|
| **David Henao Rojas** (`dahenaor-source`) | Firmware del microcontrolador, dashboard (primera versión), documentación, coordinación del repositorio | Firmware ESP32-C6 (SPI esclavo, ADC continuo, protocolo v1 y v2, calibración); pruebas unitarias del protocolo; binario de release; asignación y documentación del pinout con diagramas; dashboard HTML; robustecimiento del driver SPI y del backend (reintentos, candado, tiempos); script de diagnóstico; esta documentación |
| **Juan Camilo Giraldo** (`Giraldo0712`) | Software del SBC | Backend FastAPI simulado y real, driver SPI según el protocolo v1, servir el dashboard desde FastAPI, README del backend, scripts `test_spi.py` y `prueba_directa_spi.py`, pruebas en la BeaglePlay, cambios de velocidad SPI |
| **Daniela Barragán** | Software del SBC | _Por completar_ (no hay commits suyos a 2026-11-03) |

Tareas del enunciado y responsables:

| Tarea | Responsable(s) |
|---|---|
| Coordinación de repositorios | David Henao Rojas (_confirmar_) |
| Código y configuración de servicios en el SBC (software) | Juan Camilo Giraldo, Daniela Barragán |
| Código y pruebas preliminares del microcontrolador (firmware) | David Henao Rojas |
| Diseño de experimentos y matriz de verificación | _Por completar_ (hay una matriz inicial en [10](10-matriz-de-verificacion.md)) |
| Pruebas unitarias en SBC y microcontrolador | Microcontrolador: David (PC). SBC: _por completar_ |
| Pruebas de integración | Juan Camilo y David (con la BeaglePlay y el ESP32-C6) |

Historial de commits: `git log --format='%h %an %ad %s' --date=short`.

## 2. Declaración de uso de IA

### 2.1 Modelo y herramienta

- Herramienta: asistente de IA integrado en VS Code (GitHub Copilot, modo agente con acceso a terminal y archivos) usado por **David Henao Rojas**.
- Herramientas de IA usadas en el proyecto (declaradas por el equipo): **Copilot, Gemini y ChatGPT**. La versión exacta de cada modelo no está registrada: _por completar_ (selector de modelo de VS Code e historial de cada herramienta).
- Qué herramienta usó cada integrante y para qué tarea: _por completar_ (en este repositorio, el firmware, el dashboard y estos documentos se hicieron con el asistente de VS Code).

### 2.2 Tareas en las que se usó la IA (lado David)

| Tarea | Qué hizo la IA | Cómo se verificó |
|---|---|---|
| Firmware ESP32-C6 (SPI esclavo, ADC continuo, protocolo, presets) | Escribió el código en C y las pruebas del protocolo | Compilación con ESP-IDF 5.4.1; 7 pruebas unitarias en el PC; flasheo y arranque en la placa real; pruebas posteriores con la BeaglePlay |
| Calibración del ADC (v2) | Propuso y programó la tabla con `adc_cali_curve_fitting` tras analizar las mediciones del equipo | Compila y pasa las pruebas unitarias; **la verificación en hardware sigue pendiente** |
| Maestro SPI de referencia (`tools/spi_master_test.py`) | Escribió el script | **No se ha ejecutado en hardware** |
| Dashboard HTML/JS | Escribió la interfaz y la fuente simulada | Servido por un servidor local; probado en el navegador por el equipo con datos reales. No se pasó por un analizador de sintaxis |
| Análisis de los logs de las pruebas SPI | Interpretó las salidas de `prueba_directa_spi.py` y `diagnostico_captura.py` (conteo de válidas/inválidas, patrón alternado, flags) | Los datos los tomó el equipo en la BeaglePlay; los conteos se recalcularon sobre los textos pegados |
| Driver SPI y backend (reintentos, candado, plazos) | Reescribió partes de `spi_driver.py` y `main.py` | Esclavo SPI simulado en el PC, luego pruebas del equipo en la placa real |
| Pinout y diagramas (SVG, Mermaid y gráfico) | Generó los diagramas y corrigió la numeración mikroBUS a partir de la corrección del equipo | El equipo corrigió el error de INT (11 → 15); los SVG se renderizaron para revisarlos |
| Documentación (.md) | Redactó y organizó esta documentación | Revisión por el equipo y contraste con los archivos y logs del repositorio |

### 2.3 Cómo se verificó la salida de la IA

- **Compilación y pruebas automáticas** donde fue posible (ESP-IDF, `make test`).
- **Pruebas en hardware reales** hechas por personas (flasheo, SPI con la BeaglePlay, multímetro).
- **Corrección humana de errores de la IA**: el equipo detectó la numeración de pines mikroBUS errónea (INT) y aportó los logs que mostraron que `ARM` se perdía; el desajuste de la lectura del ADC se descubrió comparando con un multímetro.
- Lo **no verificado** se marca explícitamente como pendiente en [09](09-pruebas-y-resultados.md) y [10](10-matriz-de-verificacion.md).

### 2.4 Límites y errores de la IA conocidos en este proyecto

- Supuso un fondo de escala de 3,1 V para el ADC; la placa real no lo cumplía (ver [08](08-calibracion-adc.md)).
- Numeró mal el pin INT de mikroBUS en la primera versión.
- Los datos del AM62x (SPI maestro a 50 MHz) provienen de ejemplos de TI y **no se han confirmado** con la hoja de datos.
- Las hipótesis sobre el ruido en CH A y las transacciones perdidas son explicaciones plausibles, **no probadas**.

### 2.5 Contenido de los chats / interacciones

El enunciado pide incluir el contenido de las interacciones. **Por completar antes de la entrega:**

1. Exportar o copiar el historial de la conversación con el asistente (VS Code → vista del chat → exportar sesión, o copiar el contenido) y guardarlo en `docs/Documentación_General/chats/` (carpeta por crear).
2. Cada integrante que haya usado IA debe añadir el suyo.
3. No incluir credenciales ni datos personales en los archivos exportados.

| Integrante | Archivo del chat | Herramienta y modelo |
|---|---|---|
| David Henao Rojas | _por completar_ | Copilot (VS Code); _confirmar si también Gemini o ChatGPT_ |
| Juan Camilo Giraldo | _por completar_ | _por completar_ |
| Daniela Barragán | _por completar_ | _por completar_ |
