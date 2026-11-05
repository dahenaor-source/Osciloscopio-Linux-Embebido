# 10. Matriz de verificación

Corte de pruebas en hardware y verificaciones locales: 2026-11-05. Cada fila enlaza el requisito con la prueba y el resultado. La evidencia detallada está en [09-pruebas-y-resultados.md](09-pruebas-y-resultados.md). ✅ cumple · 🟡 parcial · ⏳ pendiente · ❌ no cumple.

## 1. Firmware (ESP32-C6)

| ID | Requisito / función | Prueba | Tipo | Resultado | Evidencia |
|---|---|---|---|---|---|
| FW-01 | Codificación y verificación de comandos | `test_cmd_*` en `firmware/tests/host` | Unitaria (PC) | ✅ Pasa | P-02 |
| FW-02 | Cabecera de estado con checksum | `test_header_*` | Unitaria (PC) | ✅ Pasa | P-02 |
| FW-03 | Presets 100/1.000/10.000/40.000 Sa/s dentro de los límites del ADC | `test_rates` | Unitaria (PC) | ✅ Pasa | P-02 |
| FW-04 | Ventanas de 64 a 1020 muestras (≥ 64) | `test_window` | Unitaria (PC) | ✅ Pasa | P-02 |
| FW-05 | Compila con ESP-IDF 5.4.1 (v1 y v2) | `idf.py build` | Compilación | ✅ Pasa | P-01, P-12 |
| FW-06 | Enlace SPI sin errores de datos (modo prueba) | Dashboard en modo de prueba; `diagnostico_captura.py` | Integración | 🟡 Una captura de 256 muestras correcta a 2 MHz (visual). Falta repetirlo de forma automática y con muchas capturas | P-06 |
| FW-07 | Cambio de preset por SPI | `POST /api/scope/config` y `fs_hz` en la cabecera | Integración | ✅ Los cuatro presets se aplicaron y verificaron por API (P-20). Desde el selector del dashboard se verificaron 100, 1.000 y 10.000 Sa/s; el preset de 40.000 no se probó desde esa interfaz | P-07, P-10, P-20 |
| FW-08 | Frecuencia de muestreo real de cada preset | Señal de frecuencia conocida o analizador lógico | Experimental | 🟡 Estimaciones con generador: 100→100,0; 1.000→1.000,09; 10.000→10.004,06; 40.000→40.307,74 Sa/s. La referencia es el valor nominal del generador; falta considerar su tolerancia/calibración | P-20 |
| FW-09 | Captura a 40 kSa/s sin desbordamiento (bonificación) | `flags` bit0 = 0 y señal conocida | Experimental | 🟡 Prueba funcional corta: 10/10 capturas sin señal y 5/5 con seno conocido de 1 kHz; todas con 256 muestras, `flags=0`, `retries=0`. Estrés prolongado fuera del alcance de esta etapa por tiempo disponible | P-10, P-20 |
| FW-10 | Velocidad máxima de SPI estable y caudal real | `--bench` a distintas velocidades | Experimental | 🟡 Caudal medido: 0,10 / 0,18 / 0,33 / 0,47 / 0,54 MB/s a 1 / 2 / 5 / 10 / 20 MHz. 20 MHz es la máxima probada, no necesariamente la máxima estable; integridad sostenida fuera del alcance de esta etapa | P-05, P-06, P-20 |
| FW-11 | Exactitud del ADC calibrado (0, 0,25 … 3 V) | Multímetro contra mV del dashboard | Experimental | 🟡 Medición preliminar v2 en CH A: 4 puntos, error absoluto 0,004–0,035 V; falta completar puntos y repeticiones. v1 tuvo error de hasta 0,72 V | P-11, P-21, [08](08-calibracion-adc.md) |
| FW-12 | Calibración del ADC por eFuse en el arranque | Log `curve-fitting (eFuse)` en el monitor serie | Experimental | ✅ Ambos canales, 2026-11-04 | P-13 |

## 2. SBC y backend

| ID | Requisito / función | Prueba | Tipo | Resultado | Evidencia |
|---|---|---|---|---|---|
| SB-01 | Abrir `/dev/spidev0.0` y leer una cabecera válida | `prueba_directa_spi.py` | Integración | ✅ Pasa (2.ª transacción) | P-04 |
| SB-02 | Captura completa (ARM → READY → lectura) | `diagnostico_captura.py`, dashboard | Integración | ✅ A 2 MHz con reintentos. ❌ a 100 kHz (HTTP 500) | P-05, P-06 |
| SB-03 | Robustez ante transacciones perdidas | Esclavo simulado en PC con pérdidas alternas | Unitaria (simulada) | ✅ Pasa (reintentos, `seq` viejo) | P-05, [06](06-backend-beagleplay.md) |
| SB-04 | Sin mezcla de transacciones entre `GET frame` y `POST config` | Solicitudes simultáneas reales de captura y cambio de preset; validar trama y configuración final | Integración | ✅ 5/5 pares simultáneos válidos en la BeaglePlay; sin trama inválida ni flags/reintentos | P-20 |
| SB-05 | Habilitar `spidev` en la mikroBUS | — | Configuración | 🟡 Funciona; método sin documentar | [03](03-hardware-y-conexiones.md) |
| SB-06 | Servir el dashboard con **apache o nginx** | `nginx -t`, estado de servicios y solicitudes locales al dashboard y `/api/scope/frame` en la BeaglePlay | Configuración/integración | ✅ `nginx -t` exitoso; Nginx y backend `active`/`enabled`; dashboard HTTP 200 y captura API válida por el proxy con 256 muestras/canal, `flags=0`, `retries=0` (P-22). La UI no se volvió a inspeccionar en navegador en esa sesión | P-16, P-18, P-22 |
| SB-07 | Arranque automático del backend (`systemd`) | Reiniciar la BeaglePlay y verificar que se pueden medir señales en el dashboard sin entrar a la terminal | Configuración | ✅ Después del reinicio el backend inició sin intervención y el dashboard permitió medir (confirmado por el usuario, 2026-11-05) | P-19 |

## 3. Dashboard

| ID | Requisito / función | Prueba | Tipo | Resultado | Evidencia |
|---|---|---|---|---|---|
| DB-01 | Muestra 2 canales y estadísticas con datos simulados | Manual en el navegador | Funcional | ✅ | [07](07-dashboard.md) |
| DB-02 | Muestra datos reales vía `/api/scope/frame` | Manual con la BeaglePlay | Integración | ✅ Modo de prueba y ADC real | P-06, P-08 |
| DB-03 | Refresco entre 0,25 y 1 Hz | Selector 0,25 / 0,5 / 1 Hz; a 1 Hz con 1 kSa/s | Funcional | 🟡 Funciona a 1 Hz con 1 kSa/s; no medido a 100 Sa/s ni a 40 kSa/s | P-08, P-09 |
| DB-04 | Cambio de preset desde el dashboard | Selector de frecuencia | Integración | ✅ 100, 1.000 y 10.000 Sa/s | P-07 |
| DB-05 | Aviso visual de overrun, comando rechazado, ADC sin calibrar, reintentos | Revisión del código | Inspección | 🟡 Implementado; no provocado en pruebas | [07](07-dashboard.md) |

## 4. Sistema completo

| ID | Requisito | Prueba | Resultado |
|---|---|---|---|
| SYS-01 | R-01: 2 señales analógicas en ventanas de adquisición | Ambos canales capturados con potenciómetro; generador usado para las pruebas de P-20 | ✅ Ambos canales entregan muestras en una ventana; en P-09 compartieron la misma fuente. No se probaron dos fuentes independientes simultáneamente | P-09, P-20 |
| SYS-02 | R-03: SPI entre procesador y microcontrolador | P-04 a P-06 | ✅ |
| SYS-03 | R-04: ≥ 64 muestras a frecuencia constante | 256 muestras por canal y estimación de fs con generador en los cuatro presets (P-20) | ✅ Requisito funcional verificado; la tolerancia del generador limita la exactitud absoluta de fs (ver FW-08) |
| SYS-04 | R-05: ≥ 3 presets cambiables por SPI | Selector del dashboard y API | ✅ 100, 1.000 y 10.000 Sa/s desde el dashboard; 40.000 Sa/s también por API (P-07, P-20) |
| SYS-05 | R-06: refresco entre 0,25 y 1 Hz | DB-03 | 🟡 |
| SYS-06 | R-07: tasa teórica de SPI y verificación experimental | [05](05-firmware-esp32c6.md) y FW-10 | 🟡 Teoría hecha; experimental parcial |
| SYS-07 | R-08: bonificación ≥ 40 kSa/s verificada | FW-09 | 🟡 15 capturas funcionales sin overrun, incluidas 5 con señal conocida; estrés prolongado fuera del alcance de esta etapa |
| SYS-08 | R-13: video del funcionamiento | — | ⏳ |

## 5. Pruebas que faltan y cómo hacerlas

| Prueba | Cómo hacerla sin analizador lógico |
|---|---|
| FW-08 frecuencia de muestreo | Estimada con generador en los cuatro presets (P-20); para cerrar, registrar modelo/tolerancia del generador o contrastar con analizador lógico |
| FW-09 40 kSa/s | Prueba funcional corta: 15 capturas válidas, 5 con seno de 1 kHz. Estrés prolongado no ejecutado y fuera del alcance de esta etapa por tiempo disponible |
| FW-10 SPI máximo y caudal | Caudal medido de 0,10–0,54 MB/s a 1–20 MHz (P-20). Prueba sostenida de integridad y búsqueda de velocidad máxima estable no ejecutadas; fuera del alcance de esta etapa por tiempo disponible |
| FW-11 ADC | Cuatro puntos iniciales comparados en P-21; completar la tabla y repetir lecturas según [08](08-calibracion-adc.md), sección 4 |
| Pérdida de transacciones | Contar transacciones inválidas por cada 1000 a distintas velocidades y con INT; añadir un contador de transacciones en el firmware |

## 6. Evidencias complementarias de verificación

Las pruebas P-01 a P-22 están registradas cronológicamente
en [09. Pruebas y resultados](09-pruebas-y-resultados.md).
Los resultados siguientes complementan las tablas de
cumplimiento anteriores sin modificar su calificación.

| Evidencia | Verificación realizada | Resultado y alcance |
|---|---|---|
| P-02 | Pruebas unitarias del protocolo en PC | Todas las pruebas ejecutadas pasaron. |
| P-15 | Lógica de selección de preset y refresco | Verificada con reloj y entorno simulados; no es una medición de temporización en hardware. |
| P-18 y P-22 | nginx, FastAPI y adquisición por API | HTTP 200; tramas válidas de 256 muestras por canal. |
| P-19 | Arranque automático del backend | Funcionamiento después de reiniciar la BeaglePlay, según la verificación registrada. |
| P-20 | Solicitudes concurrentes de captura y configuración | Diez pares completados; prueba funcional de corta duración. |
| P-20 | Captura a 40 kSa/s | Quince capturas válidas, cinco con seno conocido de 1 kHz, sin flags ni reintentos. |
| P-20 | Frecuencias de muestreo | Estimaciones de 100,0; 1.000,09; 10.004,06 y 40.307,74 Sa/s. La referencia del generador no tiene incertidumbre documentada. |
| P-20 | Caudal SPI | 0,10; 0,18; 0,33; 0,47 y 0,54 MB/s para velocidades solicitadas de 1, 2, 5, 10 y 20 MHz. No representa una medición directa de SCLK. |
| P-20 | Preset de 100 Sa/s | Una captura correcta en 2,617 s; también quedó documentada una inconsistencia anterior de selección de preset. |
| P-21 | Comparación ADC V2 y multímetro | Cuatro puntos en CH A, con error absoluto de 0,004 a 0,035 V; caracterización parcial. |

### Alcance de los resultados

Las pruebas de integración, simulación y medición
experimental se identifican por separado. Los resultados
P-20 y P-21 no constituyen certificación metrológica
completa ni prueba de estabilidad prolongada.

Los procedimientos, registros históricos y limitaciones
se conservan en la bitácora P-01 a P-22.
