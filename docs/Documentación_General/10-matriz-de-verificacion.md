# 10. Matriz de verificación

Corte de pruebas en hardware: 2026-11-04. Verificaciones locales adicionales: 2026-11-05. Cada fila enlaza el requisito con la prueba y el resultado. La evidencia detallada está en [09-pruebas-y-resultados.md](09-pruebas-y-resultados.md). ✅ cumple · 🟡 parcial · ⏳ pendiente · ❌ no cumple.

## 1. Firmware (ESP32-C6)

| ID | Requisito / función | Prueba | Tipo | Resultado | Evidencia |
|---|---|---|---|---|---|
| FW-01 | Codificación y verificación de comandos | `test_cmd_*` en `firmware/tests/host` | Unitaria (PC) | ✅ Pasa | P-02 |
| FW-02 | Cabecera de estado con checksum | `test_header_*` | Unitaria (PC) | ✅ Pasa | P-02 |
| FW-03 | Presets 100/1.000/10.000/40.000 Sa/s dentro de los límites del ADC | `test_rates` | Unitaria (PC) | ✅ Pasa | P-02 |
| FW-04 | Ventanas de 64 a 1020 muestras (≥ 64) | `test_window` | Unitaria (PC) | ✅ Pasa | P-02 |
| FW-05 | Compila con ESP-IDF 5.4.1 (v1 y v2) | `idf.py build` | Compilación | ✅ Pasa | P-01, P-12 |
| FW-06 | Enlace SPI sin errores de datos (modo prueba) | Dashboard en modo de prueba; `diagnostico_captura.py` | Integración | 🟡 Una captura de 256 muestras correcta a 2 MHz (visual). Falta repetirlo de forma automática y con muchas capturas | P-06 |
| FW-07 | Cambio de preset por SPI | Selector del dashboard y `fs_hz` en la cabecera | Integración | 🟡 100, 1.000 y 10.000 Sa/s cambian. 40.000 Sa/s con error | P-07, P-10 |
| FW-08 | Frecuencia de muestreo real de cada preset | Señal de frecuencia conocida o analizador lógico | Experimental | ⏳ Pendiente (no hay generador ni analizador) | — |
| FW-09 | Captura a 40 kSa/s sin desbordamiento (bonificación) | `flags` bit0 = 0 y señal conocida | Experimental | ❌ Error constante a 40 kSa/s, sin diagnosticar | P-10 |
| FW-10 | Velocidad máxima de SPI estable y caudal real | `--bench` a distintas velocidades | Experimental | 🟡 Funciona a 100 kHz y 2 MHz; máximo y caudal sin medir | P-05, P-06 |
| FW-11 | Exactitud del ADC calibrado (0, 0,25 … 3 V) | Multímetro contra mV del dashboard | Experimental | 🟡 Sin calibrar (v1): error hasta 0,72 V. v2 sin medir | P-11, [08](08-calibracion-adc.md) |
| FW-12 | Calibración del ADC por eFuse en el arranque | Log `curve-fitting (eFuse)` en el monitor serie | Experimental | ✅ Ambos canales, 2026-11-04 | P-13 |

## 2. SBC y backend

| ID | Requisito / función | Prueba | Tipo | Resultado | Evidencia |
|---|---|---|---|---|---|
| SB-01 | Abrir `/dev/spidev0.0` y leer una cabecera válida | `prueba_directa_spi.py` | Integración | ✅ Pasa (2.ª transacción) | P-04 |
| SB-02 | Captura completa (ARM → READY → lectura) | `diagnostico_captura.py`, dashboard | Integración | ✅ A 2 MHz con reintentos. ❌ a 100 kHz (HTTP 500) | P-05, P-06 |
| SB-03 | Robustez ante transacciones perdidas | Esclavo simulado en PC con pérdidas alternas | Unitaria (simulada) | ✅ Pasa (reintentos, `seq` viejo) | P-05, [06](06-backend-beagleplay.md) |
| SB-04 | Sin mezcla de transacciones entre `GET frame` y `POST config` | Revisión del código (`RLock`) | Inspección | 🟡 Implementado; sin prueba de concurrencia | [06](06-backend-beagleplay.md) |
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
| SYS-03 | R-04: ≥ 64 muestras a frecuencia constante | 256 muestras en el dashboard | 🟡 Frecuencia no medida con instrumento |
| SYS-04 | R-05: ≥ 3 presets cambiables por SPI | P-07 | ✅ (100, 1.000, 10.000) |
| SYS-05 | R-06: refresco entre 0,25 y 1 Hz | DB-03 | 🟡 |
| SYS-06 | R-07: tasa teórica de SPI y verificación experimental | [05](05-firmware-esp32c6.md) y FW-10 | 🟡 Teoría hecha; experimental parcial |
| SYS-07 | R-08: bonificación ≥ 40 kSa/s verificada | FW-09 | ❌ |
| SYS-08 | R-13: video del funcionamiento | — | ⏳ |

## 5. Pruebas que faltan y cómo hacerlas

| Prueba | Cómo hacerla sin analizador lógico |
|---|---|
| FW-08 frecuencia de muestreo | Señal periódica de frecuencia conocida (generador, o PWM + filtro RC generado por el propio ESP32-C6): muestras por periodo = `fs / f`. A 100 y 1.000 Sa/s también por tiempo hasta `READY` ≈ `window / fs` |
| FW-09 40 kSa/s | Capturar el mensaje de error (log del backend y monitor serie) y repetir con `flags` bit0 en muchas capturas |
| FW-10 SPI máximo y caudal | Medir con `time.perf_counter()` el tiempo de `xfer2` de 1040 bytes a 1, 2, 5, 10, 20 MHz y comparar con `bytes × 8 / f`; registrar a qué velocidad empiezan los errores |
| FW-11 ADC | Procedimiento y tabla en [08](08-calibracion-adc.md), sección 4 |
| Pérdida de transacciones | Contar transacciones inválidas por cada 1000 a distintas velocidades y con INT; añadir un contador de transacciones en el firmware |

## 6. Verificaciones locales adicionales (2026-11-05)

| Prueba | Resultado | Alcance / limitación |
|---|---|---|
| `make -C firmware/tests/host test` | ✅ Pasan todas las pruebas del protocolo | Repetición local en PC; no verifica el firmware ni el enlace SPI en hardware |
| Servir `dashboard/` con `python3 -m http.server` y solicitar `/`, `/scope.js` y `/vendor/chart.umd.js` | ✅ HTTP 200 para los tres recursos; comprobada la presencia del dashboard y de la opción API real en el HTML | Smoke test de archivos estáticos; no ejecuta Nginx, no prueba `/api/` ni reemplaza la prueba funcional en navegador |
| Prueba local de la captura real/API y validación de Nginx en la BeaglePlay | ⏳ No ejecutada en este entorno | Aquí no están disponibles `/dev/spidev0.0` ni Nginx. En la placa, P-18 confirma que la página y el proxy API responden 200 con el backend activo |
| Frecuencia de muestreo, tasa máxima SPI y exactitud de calibración (FW-08, FW-10, FW-11) | ⏳ Sin cambio | Requieren mediciones en hardware; FW-08 y FW-11 además requieren señal conocida y multímetro |

Las mediciones experimentales pendientes se mantienen abiertas; los resultados locales no se extrapolan a la BeaglePlay.
