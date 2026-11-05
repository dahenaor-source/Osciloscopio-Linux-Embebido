# 02. Requerimientos y restricciones

## 1. Requerimientos del enunciado

| ID | Requisito | Cómo se aborda | Estado |
|---|---|---|---|
| R-01 | Capturar **2 señales analógicas** en ventanas de adquisición | ADC1 del ESP32-C6, GPIO0 (CH A) y GPIO1 (CH B), captura bajo demanda | 🟡 Funciona con potenciómetro y pines al aire; sin generador de señales |
| R-02 | El SBC **sirve los datos con un dashboard por red usando apache o nginx** | Configuración de **nginx** en [`beagleplay/nginx.conf`](../../beagleplay/nginx.conf): sirve `dashboard/` en el puerto 80 y hace proxy de `/api/` a uvicorn (puerto 8000) | ✅ **Verificado en la BeaglePlay (2026-11-05):** Nginx y backend activos/habilitados; el dashboard y `/api/scope/frame` responden HTTP 200 por Nginx, y tras reiniciar la placa se pudo medir desde el dashboard sin intervención (P-18, P-19) |
| R-03 | Comunicación procesador ↔ microcontrolador **vía SPI** | BeaglePlay maestro, ESP32-C6 esclavo, modo 0, DMA | ✅ Funciona a 2 MHz (con reintentos, ver [09](09-pruebas-y-resultados.md)) |
| R-04 | Ventanas de **≥ 64 muestras** a frecuencia constante | 64 a 1020 muestras por canal; temporización por hardware del ADC | 🟡 Probado con 256; fs estimada con generador en los cuatro presets (P-20), pendiente considerar la tolerancia de la referencia |
| R-05 | **≥ 3 presets** de frecuencia, cambiables por SPI (100, 1.000, 10.000 Sa/s) | 4 presets: 100, 1.000, 10.000 y 40.000 Sa/s, comando `SET_RATE` | 🟡 100, 1.000 y 10.000 verificados desde el dashboard; 40.000 probado por API, falta confirmar selección desde dashboard |
| R-06 | Refresco del dashboard **entre 0,25 y 1 Hz** | Selector de 0,25 / 0,5 / 1 Hz; captura bajo demanda | 🟡 Funciona a 1 Hz con 1 kSa/s. A 100 Sa/s la ventana de 256 muestras dura 2,56 s, por lo que el ciclo real es ≈ 0,38 Hz (dentro del rango) |
| R-07 | **Estimar** la tasa máxima teórica de SPI y **verificarla experimentalmente** | Estimación en [05](05-firmware-esp32c6.md) (40 MHz esclavo, 50 MHz maestro ⚠️) | 🟡 Caudal medido entre 1 y 20 MHz (0,10–0,54 MB/s); falta encontrar la máxima velocidad estable y validar con analizador el reloj real |
| R-08 | Bonificación: captura **≥ 40 kSa/s** verificada | Preset 3 = 40.000 Sa/s por canal (80 kSa/s totales en el ADC) | 🟡 15/15 capturas API con 256 muestras y sin overrun (5 con seno conocido de 1 kHz); fs estimada ≈ 40.308 Sa/s. Falta prueba de estrés prolongada |
| R-09 | Grupo de **máximo 6** personas, tareas bien definidas | 3 integrantes registrados | ✅ Ver [14](14-participacion-y-uso-de-ia.md) |
| R-10 | Repositorio Git con documentación, instrucciones y código | Este repositorio | 🟡 En curso |
| R-11 | **Matriz de verificación** con resultados detallados | [10](10-matriz-de-verificacion.md) | 🟡 Parcial |
| R-12 | Declaración de participación y de uso de IA | [14](14-participacion-y-uso-de-ia.md) | ⏳ Plantilla; el equipo debe completarla |
| R-13 | **Video** del funcionamiento | — | ⏳ Pendiente |
| R-14 | Diagramas de bloques y de flujo | [01](01-descripcion-general.md) | ✅ |

### Dato adicional del docente

El profesor indicó que el SPI de la mikroBUS debe ofrecer al menos **2 Mbit/s** para sostener el flujo de muestras requerido. Referencia de cálculo: 2 canales × 2 B × 40.000 Sa/s = **160 kB/s = 1,28 Mbit/s** de datos útiles (más la cabecera y los reintentos). El enlace se probó a **2 MHz** con éxito, pero con pérdida de transacciones (ver [09](09-pruebas-y-resultados.md)).

## 2. Requerimientos propios del diseño

| ID | Requisito | Motivo |
|---|---|---|
| D-01 | Voltajes entregados en mV calibrados | El ADC sin calibrar del ESP32-C6 no es lineal ([08](08-calibracion-adc.md)) |
| D-02 | El maestro debe validar cada respuesta (magic `0xE5` y checksum) | Se pierden transacciones; sin validar, se leerían datos basura |
| D-03 | El firmware debe poder probarse **sin señal analógica** | Modo de prueba con rampas conocidas (`SET_TESTMODE`) |
| D-04 | Todo el equipo debe poder flashear sin instalar ESP-IDF | Binario en `release/` ([11](11-guia-flasheo-esp32c6.md)) |

## 3. Restricciones técnicas

| Restricción | Valor | Consecuencia |
|---|---|---|
| Rango de entrada del ADC (atenuación 12 dB) | 0 a ≈ 3,1 V, solo positivo | Offset positivo en el generador. Más de 3,3 V puede dañar el pin |
| Lógica de las placas | 3,3 V | Sin adaptadores de nivel; **no conectar +3,3 V ni +5 V entre placas**, solo GND |
| Frecuencia del ADC continuo del ESP32-C6 | 611 a 83.333 Sa/s en total | Con 2 canales alternados: máximo ≈ 41,6 kSa/s por canal; el preset de 40 kSa/s usa 80 kSa/s |
| 100 Sa/s por canal | Por debajo del mínimo del ADC | Se convierte a 2 kSa/s y se promedian 10 conversiones por muestra |
| Muestreo de 2 canales con un solo ADC | Conversión alternada | El canal B está desfasado medio periodo de conversión respecto al A |
| Tamaño de transacción SPI | 4096 B (`bufsiz` por defecto de `spidev`) | Ventana máxima de 1020 muestras por canal |
| Tiempo entre transacciones SPI | ≥ 2 ms o usar INT | El esclavo necesita rearmarse tras cada transacción |
| Sin disparo (*trigger*) | Cada captura empieza al recibir `ARM` | No hay sincronización con la señal; una señal periódica aparece con fase distinta en cada captura |
| Pines de arranque del ESP32-C6 (GPIO4, 5, 8, 9, 15) | Evitar | No se usan |
| CS nativo de SPI2 (GPIO16) | Es el TX de la UART de consola | El CS usa GPIO10 (matriz de GPIO) |
| mikroBUS del BeaglePlay | Gestionado por el driver `mikrobus` | Hay que habilitar `spidev` (⚠️ método usado por el equipo del SBC no documentado aún) |
| Sin generador de señales ni analizador lógico en el equipo (hasta ahora) | — | Se probó con potenciómetro y multímetro; la frecuencia de muestreo y el reloj SPI no se han medido con instrumento |

## 4. Presets de frecuencia

| `rate_idx` | Muestreo por canal | Muestreo del ADC (total) | Promedio | Duración de 256 muestras |
|---|---|---|---|---|
| 0 | 100 Sa/s | 2.000 | 10 conversiones | 2,56 s |
| 1 | 1.000 Sa/s | 2.000 | 1 | 256 ms |
| 2 | 10.000 Sa/s | 20.000 | 1 | 25,6 ms |
| 3 | 40.000 Sa/s | 80.000 | 1 | 6,4 ms |
