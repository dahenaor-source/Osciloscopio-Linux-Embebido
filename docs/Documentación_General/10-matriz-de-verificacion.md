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
| FW-07 | Cambio de preset por SPI | Selector del dashboard y `fs_hz` en la cabecera | Integración | 🟡 100, 1.000 y 10.000 Sa/s verificados desde el dashboard; la API entregó capturas con los cuatro presets (incluido 40.000 Sa/s), pero falta repetir la selección de 40.000 desde el dashboard | P-07, P-10, P-20 |
| FW-08 | Frecuencia de muestreo real de cada preset | Señal de frecuencia conocida o analizador lógico | Experimental | 🟡 Estimaciones con generador: 100→100,0; 1.000→1.000,09; 10.000→10.004,06; 40.000→40.307,74 Sa/s. La referencia es el valor nominal del generador; falta considerar su tolerancia/calibración | P-20 |
| FW-09 | Captura a 40 kSa/s sin desbordamiento (bonificación) | `flags` bit0 = 0 y señal conocida | Experimental | 🟡 10/10 capturas sin señal y 5/5 con seno conocido de 1 kHz; todas con 256 muestras, `flags=0`, `retries=0`. Falta una prueba prolongada/estrés | P-10, P-20 |
| FW-10 | Velocidad máxima de SPI estable y caudal real | `--bench` a distintas velocidades | Experimental | 🟡 Caudal medido: 0,10 / 0,18 / 0,33 / 0,47 / 0,54 MB/s a 1 / 2 / 5 / 10 / 20 MHz. 20 MHz es la máxima probada, no necesariamente la máxima estable; el caudal incluye sobrecarga del SO | P-05, P-06, P-20 |
| FW-11 | Exactitud del ADC calibrado (0, 0,25 … 3 V) | Multímetro contra mV del dashboard | Experimental | 🟡 Sin calibrar (v1): error hasta 0,72 V. v2 sin medir | P-11, [08](08-calibracion-adc.md) |
| FW-12 | Calibración del ADC por eFuse en el arranque | Log `curve-fitting (eFuse)` en el monitor serie | Experimental | ✅ Ambos canales, 2026-11-04 | P-13 |

## 2. SBC y backend

| ID | Requisito / función | Prueba | Tipo | Resultado | Evidencia |
|---|---|---|---|---|---|
| SB-01 | Abrir `/dev/spidev0.0` y leer una cabecera válida | `prueba_directa_spi.py` | Integración | ✅ Pasa (2.ª transacción) | P-04 |
| SB-02 | Captura completa (ARM → READY → lectura) | `diagnostico_captura.py`, dashboard | Integración | ✅ A 2 MHz con reintentos. ❌ a 100 kHz (HTTP 500) | P-05, P-06 |
| SB-03 | Robustez ante transacciones perdidas | Esclavo simulado en PC con pérdidas alternas | Unitaria (simulada) | ✅ Pasa (reintentos, `seq` viejo) | P-05, [06](06-backend-beagleplay.md) |
| SB-04 | Sin mezcla de transacciones entre `GET frame` y `POST config` | Solicitudes simultáneas reales de captura y cambio de preset; validar trama y configuración final | Integración | ✅ 5/5 pares simultáneos válidos en la BeaglePlay; sin trama inválida ni flags/reintentos | P-20 |
| SB-05 | Habilitar `spidev` en la mikroBUS | — | Configuración | 🟡 Funciona; método sin documentar | [03](03-hardware-y-conexiones.md) |
| SB-06 | Servir el dashboard con **apache o nginx** | `nginx -t`, `systemctl status nginx` y `curl -i http://127.0.0.1/` en la BeaglePlay; `/api/scope/frame` por el proxy con el backend encendido | Configuración | ✅ nginx activo y habilitado; dashboard y API responden HTTP 200 a través de Nginx en la BeaglePlay (2026-11-05). `nginx -t` no se pudo repetir en esta sesión por fallo de autenticación de `sudo`; consta previamente exitoso en [06](06-backend-beagleplay.md) | P-16, P-18 |
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
| SYS-01 | R-01: 2 señales analógicas en ventanas de adquisición | Potenciómetro en CH A y CH B (DC) | 🟡 DC verificado; sin señal periódica |
| SYS-02 | R-03: SPI entre procesador y microcontrolador | P-04 a P-06 | ✅ |
| SYS-03 | R-04: ≥ 64 muestras a frecuencia constante | 256 muestras en el dashboard y estimación con generador en los cuatro presets (P-20) | 🟡 Se estimó fs en los cuatro presets; falta incluir la tolerancia calibrada del generador |
| SYS-04 | R-05: ≥ 3 presets cambiables por SPI | P-07 | ✅ (100, 1.000, 10.000) |
| SYS-05 | R-06: refresco entre 0,25 y 1 Hz | DB-03 | 🟡 |
| SYS-06 | R-07: tasa teórica de SPI y verificación experimental | [05](05-firmware-esp32c6.md) y FW-10 | 🟡 Teoría hecha; experimental parcial |
| SYS-07 | R-08: bonificación ≥ 40 kSa/s verificada | FW-09 | 🟡 15 capturas sin overrun, incluidas 5 con señal conocida; falta estrés prolongado |
| SYS-08 | R-13: video del funcionamiento | — | ⏳ |

## 5. Pruebas que faltan y cómo hacerlas

| Prueba | Cómo hacerla sin analizador lógico |
|---|---|
| FW-08 frecuencia de muestreo | Estimada con generador en los cuatro presets (P-20); para cerrar, registrar modelo/tolerancia del generador o contrastar con analizador lógico |
| FW-09 40 kSa/s | 15 capturas válidas, 5 con seno de 1 kHz; repetir bajo carga prolongada y conservar log/flags |
| FW-10 SPI máximo y caudal | Caudal medido de 0,10–0,54 MB/s a 1–20 MHz (P-20); validar respuestas de las transferencias y probar límites superiores/errores |
| FW-11 ADC | Procedimiento y tabla en [08](08-calibracion-adc.md), sección 4 |
| Pérdida de transacciones | Contar transacciones inválidas por cada 1000 a distintas velocidades y con INT; añadir un contador de transacciones en el firmware |

## 6. Verificaciones locales adicionales (2026-11-05)

| Prueba | Resultado | Alcance / limitación |
|---|---|---|
| `make -C firmware/tests/host test` | ✅ Pasan todas las pruebas del protocolo | Repetición local en PC; no verifica el firmware ni el enlace SPI en hardware |
| Servir `dashboard/` con `python3 -m http.server` y solicitar `/`, `/scope.js` y `/vendor/chart.umd.js` | ✅ HTTP 200 para los tres recursos; comprobada la presencia del dashboard y de la opción API real en el HTML | Smoke test de archivos estáticos; no ejecuta Nginx, no prueba `/api/` ni reemplaza la prueba funcional en navegador |
| Prueba local de la captura real/API y validación de Nginx en la BeaglePlay | ⏳ No ejecutada en este entorno | Aquí no están disponibles `/dev/spidev0.0` ni Nginx. En la placa, P-18 confirma que la página y el proxy API responden 200 con el backend activo |
| Exactitud de calibración ADC v2 (FW-11) | ⏳ Sin cambio | Requiere comparar el promedio del dashboard con el multímetro en varios voltajes |

## 7. Pruebas reales adicionales en la BeaglePlay (2026-11-05)

| Prueba | Resultado | Alcance / limitación |
|---|---|---|
| Concurrencia `GET /api/scope/frame` + `POST /api/scope/config` | ✅ 5/5 pares simultáneos al cambiar entre 1 y 10 kSa/s; tramas de 256 muestras válidas, sin flags ni reintentos, y el preset solicitado quedó aplicado al terminar cada par | Prueba de integración por la API real; no sustituye una prueba prolongada con carga de varios clientes |
| Capturas a 40 kSa/s | ✅ 10/10 HTTP 200 sin señal y otras 5/5 con seno conocido de 1 kHz; 256 muestras, `flags=0`, `retries=0` | No sustituye una prueba prolongada de estrés |
| Frecuencia de muestreo por cruces ascendentes de una senoide | 🟡 100 Sa/s: 100,0; 1 kSa/s: 1.000,09; 10 kSa/s: 10.004,06; 40 kSa/s: 40.307,74 Sa/s | Generador nominal de 6,1 / 164 / 1.000 Hz, respectivamente; falta incluir tolerancia calibrada del generador |
| Benchmark SPI `tools/spi_master_test.py --bench` | 🟡 0,10 / 0,18 / 0,33 / 0,47 / 0,54 MB/s a 1 / 2 / 5 / 10 / 20 MHz | 20 MHz fue el máximo probado; el benchmark mide caudal efectivo con sobrecarga, no el reloj con analizador lógico |
| Captura a 100 Sa/s | ✅ HTTP 200, preset reportado 100 Sa/s, 256 muestras por canal, `flags=0`, `retries=0`, respuesta en 2,617 s | Latencia total de API, no medición instrumental de la frecuencia real del ADC |
| Regresión de presets y restauración | ✅ Capturas válidas a 1 y 10 kSa/s; al terminar se restauró el preset habitual de 1 kSa/s | Una de tres capturas del primer bloque a 100 Sa/s respondió con preset 1; la prueba aislada posterior a 100 Sa/s sí coincidió con preset 0 |
| Exactitud de calibración ADC v2 (FW-11) | ⏳ Pendiente | Requiere comparar el promedio del dashboard con el multímetro en varios voltajes |

FW-08 y FW-10 tienen mediciones experimentales parciales; FW-11 sigue pendiente. Las pruebas de P-20 se ejecutaron contra la BeaglePlay accesible por USB.
