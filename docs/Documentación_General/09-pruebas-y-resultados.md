# 09. Pruebas y resultados

Bitácora cronológica de las pruebas, con la evidencia disponible. Lo que no se ejecutó en hardware se indica de forma explícita. Las pruebas con la BeaglePlay las hizo el equipo del SBC (Juan Camilo); las de compilación y las del PC, David Henao Rojas.

## Resumen

Los resultados P-05, P-06, P-08, P-10, P-11 y P-14 que siguen reflejan lo observado en sus fechas originales; no se sustituyen retrospectivamente por los resultados posteriores. Las pruebas posteriores citadas permiten actualizar la evaluación del estado actual, pero no equivalen a repetir íntegramente cada diagnóstico histórico.

| ID | Fecha | Prueba | Resultado |
|---|---|---|---|
| P-01 | 2026-10-28 | Compilar, flashear y arrancar el firmware v1 | ✅ |
| P-02 | 2026-10-28 | Pruebas unitarias del protocolo en el PC | ✅ 7 pruebas |
| P-03 | 2026-10-30 | Revisión del pinout mikroBUS | ✅ Corregido INT 11 → 15 |
| P-04 | 2026-10-30 | Primera lectura SPI desde la BeaglePlay (100 kHz) | ✅ Cabecera válida en la 2.ª transacción |
| P-05 | 2026-10-30 | Diagnóstico de captura completa (100 kHz y 2 MHz) | 🟡 Prueba histórica: ≈ 50 % de transacciones inválidas y `ARM` perdido. La mitigación y capturas válidas a 2 MHz se verificaron después (P-06, P-18, P-20, P-22); causa raíz y estabilidad prolongada no determinadas |
| P-06 | 2026-10-30 | Backend con reintentos: modo de prueba en el dashboard | ✅ Captura y datos de prueba válidos a 2 MHz; ❌ 100 kHz no completó la captura y no es la velocidad operativa |
| P-07 | 2026-10-30 | Preset del dashboard contra el del ESP32-C6 | ✅ Corregido |
| P-08 | 2026-10-30 | ADC real con potenciómetro | 🟡 Lecturas reales; se observó ruido periódico en CH A. P-09 mostró señal plana tras añadir condensador y evitar tráfico durante el muestreo; la causa del ruido no quedó aislada |
| P-09 | 2026-10-30 | Condensador en la entrada | ✅ Señal plana y estable |
| P-10 | 2026-10-30 | Presets de 10.000 y 40.000 Sa/s | 🟡 Fallos históricos: checksum esporádico a 10 kSa/s y error a 40 kSa/s. En P-20 no se reportaron en las capturas válidas de medición; 40 kSa/s pasó 15/15 capturas. Operación prolongada no evaluada |
| P-11 | 2026-10-30 | Exactitud del ADC v1 sin calibrar | 🟡 v1 tuvo hasta 0,72 V de error; P-21 midió v2 en cuatro puntos de CH A con error absoluto máximo de 0,035 V. El problema de v1 fue atendido con calibración eFuse; falta completar la caracterización v2 |
| P-12 | 2026-11-01 | Firmware v2 con calibración | ✅ Compila (11-01); ✅ arranca en hardware con calibración de eFuse (P-13) |
| P-13 | 2026-11-04 | Arranque del firmware v2 en una ESP32-C6 real | ✅ Arranca sin reinicios; calibración eFuse en ambos canales |
| P-14 | 2026-11-04 | SPI sin respuesta tras conectar el generador | 🟡 Diagnóstico devolvió todo `00`; la ruta SPI volvió a operar en capturas API posteriores (P-18, P-20, P-22), pero no se identificó la causa raíz de ese síntoma |
| P-15 | 2026-11-05 | Dashboard: preset enviado al iniciar y un solo bucle de sondeo | ✅ Lógica verificada con simulación de reloj virtual; ⏳ falta probarlo en el navegador con el hardware |
| P-16 | 2026-11-05 | nginx en la BeaglePlay | ✅ Activo, habilitado y sirviendo el dashboard (HTTP 200); 🟡 proxy `/api/` con 502 por backend apagado |
| P-17 | 2026-11-05 | Servicio systemd del backend (arranque automático) | ✅ Servicio activo y habilitado; tras reiniciar la placa el dashboard volvió a medir sin intervención (P-19) |
| P-18 | 2026-11-05 | nginx y API real con el backend activo en la BeaglePlay | ✅ Nginx y backend activos/habilitados; dashboard y `/api/scope/frame` responden HTTP 200 |
| P-19 | 2026-11-05 | Arranque del backend tras reiniciar la BeaglePlay | ✅ Inició sin entrar a la terminal y se pudo medir desde el dashboard (confirmado por el usuario) |
| P-20 | 2026-11-05 | Concurrencia API, frecuencia de muestreo, benchmark SPI y capturas a 40 kSa/s | ✅ Concurrencia válida; frecuencias estimadas en los cuatro presets; caudal SPI medido 1–20 MHz; 15/15 capturas a 40 kSa/s sin flags ni reintentos |
| P-21 | 2026-11-05 | Comparación preliminar ADC v2 con potenciómetro y multímetro (CH A) | 🟡 4 puntos; error absoluto 0,004–0,035 V; caracterización completa pendiente |
| P-22 | 2026-11-05 | Revalidación de Nginx y captura real/API en la BeaglePlay | ✅ Nginx y backend activos/habilitados; configuración válida; dashboard HTTP 200; API devolvió trama válida de 256 muestras/canal, `flags=0`, `retries=0` |

## P-01. Compilación, flasheo y arranque (firmware v1)

- Entorno: WSL Ubuntu, ESP-IDF 5.4.1, `usbipd` para el USB, `/dev/ttyACM0`.
- `idf.py build`: correcto. Se flasheó la ESP32-C6-DevKitC-1 real.
- Monitor serie: `scope_link: SPI esclavo listo (SCLK=6 MOSI=7 MISO=2 CS=10 HS=3)`.

## P-18. nginx y API real con el backend activo en la BeaglePlay

Prueba ejecutada en la terminal de `camilo@BeagleBone` el 2026-11-05. Las comprobaciones de estado y `curl` se completaron aunque `sudo nginx -t` no pudo ejecutarse porque la contraseña de `sudo` se rechazó tres veces.

```
systemctl is-active nginx                 -> active
systemctl is-enabled nginx                -> enabled
systemctl is-active osciloscopio-backend  -> active
systemctl is-enabled osciloscopio-backend -> enabled
curl -i http://127.0.0.1/                 -> HTTP/1.1 200 OK, Server: nginx
curl -i http://127.0.0.1/api/scope/frame  -> HTTP/1.1 200 OK, application/json
```

La respuesta API incluyó `seq: 4354`, `rate_idx: 1`, `fs_hz: 1000`, `window: 256`, `flags: 0`, `retries: 0` y 256 muestras para cada canal. Esto confirma en esa ejecución que Nginx sirve el dashboard y proxya correctamente al backend, que capturó y devolvió una trama. No valida por sí sola la exactitud del ADC ni el arranque automático después de reiniciar la placa.

## P-02. Pruebas unitarias del protocolo (PC)

`make -C firmware/tests/host test` → `OK: todas las pruebas del protocolo pasaron` (comandos, cabecera con checksum y corrupción, presets, ventanas). Se repitió con el firmware v2.

## P-03. Corrección del pinout

La numeración de mikroBUS de la primera versión ponía INT en el pin 11 (en realidad SDA). El equipo señaló que INT es el **15**. Se adoptó la numeración en U, se añadió la tabla de 16 pines y se rehicieron los diagramas ([03](03-hardware-y-conexiones.md)). Se descargó la especificación mikroBUS v2.0; define solo el orden y los nombres de la serigrafía, no numera los pines. ⚠️ No contrastado con el esquemático de la BeaglePlay.

## P-04. Primera lectura SPI (`prueba_directa_spi.py`, 100 kHz, modo 0)

Se envió `NOP` = `00 00 00 5A` más 12 ceros, 5 intentos:

```
PRUEBA 1  RX: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF     -> sin respuesta válida
PRUEBA 2  RX: E5 00 00 01 00 01 00 00 E8 03 00 00 02 0C 01 01     -> SPI OK: cabecera válida
```

Decodificación de la 2.ª: `magic E5`, estado `IDLE`, flags 0, preset 1, ventana `0x0100` = 256, `seq` 0, `fs_hz` `0x03E8` = 1000, 2 canales, 12 bits, versión 1, checksum `01`.

**Conclusión:** cableado, pinmux y `/dev/spidev0.0` funcionan. Pero la primera transacción devolvió todo `0xFF`: ya aparece la pérdida de transacciones.

## P-05. Diagnóstico de captura (`diagnostico_captura.py`)

Secuencia: NOP, RESET, NOP, SET_RATE, NOP, TESTMODE, NOP, ARM y 400 sondeos con NOP cada 10 ms.

| Velocidad | Sondeos válidos | Sondeos inválidos | Patrón | `READY` alcanzado |
|---|---|---|---|---|
| 100 kHz | 178 de 400 | 222 | Casi alternado (`oFoFFoFoFo…`) | No |
| 2 MHz | 200 de 400 | 200 | **Exactamente alternado** (`FoFoFo…`) | No |

Observaciones:

- Dos tipos de respuesta inválida: todo `0xFF`, y `0xFF…` seguido de bytes de cabecera desplazados (por ejemplo `ff×11 94 00 00 04 00`), es decir, el esclavo se armó a mitad de la transacción.
- Tras una transacción corrupta, la cabecera siguiente traía el flag **bit2** (checksum de comando inválido): el esclavo recibió un comando basura y lo descartó.
- El `ARM` cayó en una transacción inválida y no se ejecutó. El estado siguió en `IDLE` los 400 sondeos. Por eso `capture_frame` original devolvía error (HTTP 500).
- La cabecera válida es correcta incluso a 2 MHz: el problema no es la velocidad ni el cableado.
- **Causa raíz no identificada.** Hipótesis por verificar: tiempo que tarda el esclavo en volver a armar la transacción tras cada una (`spi_slave_transmit` con cola de 1), integridad de señal en cables sueltos, o el comportamiento de CS en `spidev`.

**Respuesta aplicada (en el maestro):** reintentos hasta cabecera válida, candado, comprobación de `seq`, plazo calculado ([06](06-backend-beagleplay.md)). El esclavo no se modificó. En las capturas posteriores del backend a 2 MHz hubo tramas completas válidas (P-06, P-18, P-20 y P-22), pero no se repitió el barrido diagnóstico de 400 sondeos ni se identificó la causa raíz. Por tanto, la pérdida inicial queda como observación histórica; la mitigación funcional está verificada, no la ausencia absoluta de pérdidas.

## P-06. Backend con reintentos y dashboard

- **2 MHz:** sin errores en el servidor. El dashboard mostró las rampas del modo de prueba: CH A de 0 a 0,193 V y CH B de 3,100 a 2,907 V, que coincide con `i` y `4095 − i` (captura en [07](07-dashboard.md)). Valida el SPI, la cabecera, el desempaquetado y la API de extremo a extremo.
- **100 kHz:** el dashboard seguía recibiendo `HTTP 500` (`[ERROR REST] Falla al capturar trama`). Con tantas transacciones perdidas a baja velocidad, los 12 reintentos por transacción no bastaron. Se adopta **2 MHz** como velocidad de trabajo, que además cumple los 2 Mbit/s que pidió el docente. Las verificaciones posteriores se realizaron a la velocidad operativa de 2 MHz; 100 kHz no es la configuración soportada.

## P-07. Preset desincronizado

La barra de estado decía `100 Sa/s · ventana 2560 ms` con el selector en 1.000 Sa/s: el dashboard nunca enviaba el preset al cargar y el ESP32-C6 conservaba el anterior. Corrección: si `rate_idx` de la trama difiere del seleccionado, el dashboard lo reenvía ([07](07-dashboard.md)). Verificado después con los cambios de preset en el dashboard.

## P-08. ADC real y potenciómetro

- Sin señal: ≈ 0,12 V con ruido (pines flotantes). El firmware salió del modo de prueba al reiniciar.
- Con potenciómetro (sin condensador): CH B estable en 2,556 V y **CH A con caídas periódicas** cada ≈ 26 ms hasta 0,666 V que se recuperaban exponencialmente; 39 reintentos SPI en esa captura (a 1 kSa/s).
- **Hipótesis** (no confirmada): pulsos acoplados desde el tráfico SPI a una entrada de alta impedancia. Mitigación aplicada: el backend no consulta por SPI durante el muestreo.
- En P-09, al añadir un condensador y evitar tráfico durante el muestreo, ambos canales quedaron planos (pico-pico 0,001 V). Como se cambiaron dos condiciones a la vez, no se aisló cuál mitigó el ruido. P-21 volvió a mostrar lecturas de CH A comparables con el multímetro en cuatro niveles; no incluyó una medición de ruido pico-pico. La causa del ruido periódico original sigue sin confirmarse y no hay evidencia de que continúe como fallo funcional.

## P-09. Condensador de filtrado

Con un condensador entre el cursor y GND (CH A en el cursor; CH B en el otro extremo de la resistencia de 1 kΩ), a 10.000 Sa/s: **ambas líneas planas en 2,556 V**, pico-pico 0,001 V, 24 reintentos SPI. Como se aplicaron a la vez la espera sin tráfico y el condensador, **no se sabe cuál de los dos eliminó el ruido**. Ambos canales leen lo mismo por estar en el mismo nodo.

![Con condensador](img/dashboard-con-condensador.png)

## P-10. Presets de 10.000 y 40.000 Sa/s

- A **100 y 1.000 Sa/s** todo correcto.
- A **10.000 Sa/s** el dashboard mostró avisos esporádicos de «checksum de comando» (el flag bit2 pegado de una transacción perdida). Se enmascaró el flag y se expone `retries`. En las capturas posteriores de P-20 usadas para estimar la frecuencia no se informaron flags ni reintentos; el aviso original no se reproduce en esa evidencia, aunque no se hizo una prueba prolongada.
- A **40.000 Sa/s** se observó entonces un error constante, sin guardar el mensaje exacto. En P-20 se obtuvieron 15/15 capturas válidas (10 sin señal y 5 con seno conocido de 1 kHz), con 256 muestras/canal, `flags=0` y `retries=0`, y se estimó `fs ≈ 40.308 Sa/s`. El fallo histórico no se reprodujo en esas capturas. No se ejecutó el estrés prolongado; la causa original no se determinó.

## P-11. Exactitud del ADC sin calibrar

Multímetro contra dashboard (firmware v1): 0,50→0,70 V, 1,30→1,00 V, 2,00→1,80 V, 3,28→2,556 V. Respuesta **no lineal**, error hasta 0,72 V ([08](08-calibracion-adc.md)). El dashboard no limitaba el valor: el dato crudo ya era bajo. Este fallo corresponde a la conversión lineal de v1; el firmware v2 introdujo calibración eFuse y P-21 registró una comparación inicial de cuatro niveles (error absoluto máximo 0,035 V). No se considera que el problema de exactitud de v1 siga sin atender, aunque la caracterización del v2 aún es parcial.

## P-12. Firmware v2

Calibración por eFuse con tabla de 4096 entradas por canal; protocolo v2 (mV). Compila con ESP-IDF 5.4.1 y las pruebas unitarias pasan (2026-11-01). El arranque en hardware se registra en P-13.

## P-13. Arranque del firmware v2 en hardware (2026-11-04)

Un integrante compiló el firmware desde el repositorio (versión `b3b4318`, compilado el 2026-11-04 12:36 con ESP-IDF 5.4.1), lo flasheó en una ESP32-C6 y abrió el monitor serie (`miniterm`, 115200). Salida relevante:

```
I (250) main: Osciloscopio ESP32-C6: esclavo SPI + ADC continuo
I (269) capture: canal 0: calibracion curve-fitting (eFuse)
I (285) capture: canal 1: calibracion curve-fitting (eFuse)
I (285) gpio: GPIO[3]| InputEn: 0| OutputEn: 1| ...
I (288) scope_link: SPI esclavo listo (SCLK=6 MOSI=7 MISO=2 CS=10 HS=3)
I (294) main_task: Returned from app_main()
```

Conclusiones:
- El firmware v2 **arranca sin reinicios ni errores** (descarta un desbordamiento de pila por la tabla de calibración).
- **Esa placa sí trae los datos de calibración en la eFuse** (esquema curve-fitting en ambos canales). Revisión del chip v0.2, eFuse v0.3.
- Se confirma la asignación de pines SPI y que INT = GPIO3 es una salida.
- La medición preliminar posterior está registrada en P-21: cuatro puntos de CH A con error absoluto máximo observado de 0,035 V. Faltan niveles, repeticiones, el canal B y la incertidumbre del multímetro para caracterizar la exactitud.
- Observación menor: el chip detectado tiene 16 MB de flash y el firmware usa 4 MB (el arranque lo avisa y usa 4 MB); no afecta.

## P-14. SPI sin respuesta tras conectar el generador (2026-11-04, en diagnóstico)

Con el backend a 100 kHz (valor por defecto antiguo): `[ERROR REST] sin cabecera valida antes de ARM` (HTTP 500). Con `prueba_directa_spi.py` las 5 pruebas devolvieron **todos los bytes `00`** (el 2026-10-30 devolvían `FF` en las tramas perdidas y una cabecera válida en otras). Pulsar RESET en el ESP32-C6 no lo corrigió. El monitor serie (P-13) mostró que el firmware estaba activo y el esclavo SPI iniciado; se sospechó del cableado de MISO, GND o CS, o de la configuración de la BeaglePlay. `00` fijo apuntaba a MISO a nivel bajo. Se propusieron pruebas de aislamiento, pero no consta que se realizaran.

**Reevaluación:** las capturas válidas posteriores por el backend a 2 MHz (P-18, P-20 y P-22) demuestran que la ruta SPI volvió a operar y que el síntoma `00` fijo no persistió en esas sesiones. No se identificó qué condición transitoria lo causó ni se repitieron las pruebas de aislamiento. Se preparó un firmware diagnóstico con contador de transacciones, pero no consta que se haya utilizado para aislar esta causa.

## P-15. Dashboard: preset al iniciar y bucle único (2026-11-05)

Dos fallos observados con la rama `Osc-V2` y un video del equipo (40.000 Sa/s, refresco de 4 s): (1) al pulsar Start con «API real» el dashboard se quedaba en «ajustando frecuencia...» hasta que se cambiaba la frecuencia a mano, y (2) el cuadro cambiaba **cada 2 s** en vez de cada 4 s. Causas halladas en `scope.js`: (1) no se enviaba el preset al ESP32-C6, que conserva el último recibido; (2) cada cambio de ajuste hacía `stop(); start()` y el bucle anterior seguía vivo, así que se acumulaban bucles. Corrección en la rama `Osc-V3` ([07](07-dashboard.md), sección 4).

Verificación sin hardware (Node.js con DOM y red simulados y reloj virtual): código anterior = «ajustando frecuencia...» con 0 envíos de preset en el primer arranque y 20 capturas en 20 s con refresco de 4 s tras cambiar ajustes; código nuevo = «capturando» con el preset enviado y una captura exacta cada 4000 ms. **Pendiente** probarlo en un navegador con la placa. El equipo confirmó que `Osc-V3` es la última versión en la que funcionó todo.

## P-16. nginx en la BeaglePlay (2026-11-05)

`sudo nginx -t` correcto; `systemctl status nginx` → `enabled`, `active (running)` desde hacía 16 h; `curl -i http://127.0.0.1/` → `200 OK`, `Server: nginx`, con el `index.html` del dashboard. `curl -i http://127.0.0.1/api/scope/frame` → `502 Bad Gateway`: el proxy no encontró el backend porque estaba apagado (se arrancaba a mano). Ver [06](06-backend-beagleplay.md) y [13](13-pendientes-y-riesgos.md), A-1.

## P-17. Servicio systemd del backend (2026-11-05)

Para que al alimentar la placa todo arranque solo: unidad [`beagleplay/systemd/osciloscopio-backend.service`](../../beagleplay/systemd/osciloscopio-backend.service) e instalador [`beagleplay/instalar_servicio.sh`](../../beagleplay/instalar_servicio.sh) ([06](06-backend-beagleplay.md), sección 3). Verificado en el PC de desarrollo: `bash -n` del script, `systemd-analyze verify` de la unidad renderizada (sin advertencias) y negativa del script a ejecutarse sin `sudo`. Se corrigió además `spi_driver.py`, que no reintentaba abrir `/dev/spidev0.0` si no existía al arrancar (cinco pasos probados con un `spidev` simulado: arranque sin dispositivo, aparición posterior, `OSError` en `xfer2` y recuperación). En P-18 se confirmó que `osciloscopio-backend` está `active` y `enabled` en la BeaglePlay.

## P-19. Arranque del backend tras reiniciar la BeaglePlay (2026-11-05)

Según confirmó el usuario, después de reiniciar la placa el backend arrancó automáticamente, sin necesidad de entrar a la terminal, y el dashboard permitió medir. Esto verifica SB-07 y la ruta normal de arranque; no constituye una prueba de concurrencia de solicitudes `GET` y `POST`.

## P-20. Concurrencia API, presets y captura a 40 kSa/s (2026-11-05)

Pruebas ejecutadas contra la BeaglePlay por la interfaz USB (`192.168.7.2`), a través de Nginx:

- **Concurrencia:** se lanzaron 5 pares simultáneos de `GET /api/scope/frame` y `POST /api/scope/config` al mismo preset de 1 kSa/s: 5/5 pares respondieron correctamente con trama completa (256 muestras por canal), `flags=0` y `retries=0`. Luego se hicieron otros 5 pares simultáneos alternando el cambio entre 1 y 10 kSa/s: 5/5 capturas fueron válidas, sin flags ni reintentos; después de cada par una captura serial confirmó que quedó aplicado el preset solicitado. Esta prueba integra las dos rutas y valida el acceso compartido al SPI; no es una prueba prolongada con varios clientes.
- **Presets y frecuencia:** se estimó `fs` a partir de cruces ascendentes de una senoide del generador en las muestras del ADC. Fórmula: `fs_estimado = muestras_por_periodo × frecuencia_generador`.

  | Preset | Canal / señal | Trama | Muestras por periodo | `fs` estimado | Diferencia vs nominal |
  |---:|---|---:|---:|---:|---:|
  | 100 Sa/s | CH B, generador 6,1 Hz | 256 muestras | 16,3934 | 100,0 Sa/s | 0,00 % |
  | 1.000 Sa/s | CH A, generador 164 Hz | 8 tramas, 56 intervalos | 6,09814 | 1.000,09 Sa/s | +0,009 % |
  | 10.000 Sa/s | CH A, generador 164 Hz | 3 tramas, 10 intervalos | 61,00039 | 10.004,06 Sa/s | +0,041 % |
  | 40.000 Sa/s | CH A, generador 1.000 Hz | 5 tramas, 26 intervalos | 40,30774 | 40.307,74 Sa/s | +0,769 % |

  Las señales se configuraron a 1,5 Vpp y offset de 1,5 V. Las tramas válidas marcaron `flags=0`, `retries=0`, con 256 muestras por canal. Las estimaciones usan la frecuencia nominal indicada por el generador; no se conoce su tolerancia/calibración. Una captura aislada a 100 Sa/s tardó 2,617 s. En el primer bloque a 100 Sa/s, 2 de 3 capturas coincidieron y la tercera devolvió 1 kSa/s, por lo que no se usa como medición válida.
- **40 kSa/s:** además de las 10 capturas consecutivas sin señal (HTTP 200, `flags=0`, `retries=0`, 50–51 ms), se hicieron 5 capturas con seno conocido de 1 kHz; todas devolvieron HTTP 200, secuencias consecutivas, 256 muestras por canal, `flags=0` y `retries=0`. El error constante reportado en P-10 no se reprodujo; falta una prueba de estrés prolongada.
- **Estado final:** se restauró el preset 1 kSa/s y una captura final confirmó `rate_idx=1`, `fs_hz=1000`, `flags=0`, `retries=0`.
- **Regresión local:** `make -C firmware/tests/host test` pasó (`OK: todas las pruebas del protocolo pasaron`).
- **Benchmark SPI:** repetido con `sudo` para `tools/spi_master_test.py --bench`; caudales medidos:

  | Reloj solicitado | Caudal medido |
  |---:|---:|
  | 1 MHz | 0,10 MB/s |
  | 2 MHz | 0,18 MB/s |
  | 5 MHz | 0,33 MB/s |
  | 10 MHz | 0,47 MB/s |
  | 20 MHz | 0,54 MB/s |

  Cada ejecución inició con una captura completa; el benchmark mide 200 transferencias de 1040 bytes con sobrecarga del SO. 20 MHz es la máxima velocidad probada, no una certificación del reloj real ni de la máxima velocidad estable. Al terminar se ejecutó el script sin `--bench` para apagar el modo de prueba y restaurar el preset 1 kSa/s.
- **Estado posterior:** la exactitud del ADC v2 tiene una comparación preliminar en P-21. Las pruebas prolongadas de integridad SPI y de 40 kSa/s no se ejecutarán en esta etapa por tiempo disponible; los datos cortos no certifican estabilidad sostenida.

## P-21. Comparación preliminar del ADC v2 con multímetro (2026-11-05)

El usuario comparó el voltaje medido con multímetro y el valor **Prom** del dashboard con el potenciómetro conectado a CH A. Se calcula `error = dashboard − multímetro`; el error relativo usa el valor absoluto del error dividido por la lectura del multímetro. En 0 V no aplica el porcentaje.

| Multímetro (V) | Dashboard (V) | Error (V) | Error relativo |
|---:|---:|---:|---:|
| 0,000 | 0,004 | +0,004 | N/A |
| 0,519 | 0,525 | +0,006 | 1,16 % |
| 1,537 | 1,557 | +0,020 | 1,30 % |
| 2,996 | 3,031 | +0,035 | 1,17 % |

En los cuatro puntos el error absoluto máximo observado fue 0,035 V. El error relativo de los tres puntos distintos de cero fue 1,16–1,30 %. Resultado **parcial**: no se informaron repeticiones, incertidumbre/modelo del multímetro ni lecturas del canal CH B; faltan más niveles y mediciones repetidas antes de concluir sobre la exactitud.

## P-22. Revalidación de Nginx y API real en la BeaglePlay (2026-11-05)

El usuario ejecutó los comandos desde la BeaglePlay:

- `sudo nginx -t`: configuración sintáctica correcta y prueba exitosa.
- `systemctl is-active/is-enabled nginx`: `active` / `enabled`.
- `systemctl is-active/is-enabled osciloscopio-backend`: `active` / `enabled`.
- Solicitud al dashboard local de Nginx: HTTP 200.
- `GET /api/scope/frame` a través del proxy de Nginx: JSON válido con `seq=1635`, `rate_idx=1`, `fs_hz=1000`, 256 muestras en cada canal, `flags=0` y `retries=0`. El script comprobó que ambos canales tuvieran la misma cantidad de muestras y que la ventana fuera de al menos 64.

**Resultado:** pasó la validación del servicio/configuración de Nginx y una captura real de la API/backend por el proxy. No se repitió la comprobación visual en el navegador en esta sesión; la prueba por `curl` no la sustituye.

## Hallazgos principales

1. Históricamente el enlace SPI perdió cerca de la mitad de transacciones (P-05); en las capturas y pruebas API de P-18/P-20 no hubo reintentos. La integridad prolongada no se evaluará en esta etapa por tiempo disponible.
2. El maestro debe validar y reintentar siempre; con eso la captura completa funciona.
3. El ADC v1 sin calibrar tuvo errores de hasta 0,72 V (P-11). La comparación preliminar del v2 obtuvo errores de 0,004–0,035 V en cuatro puntos de CH A (P-21), pero aún no caracteriza la exactitud.
4. El preset de 40 kSa/s produjo 15/15 capturas válidas, incluidas 5 con seno conocido; la prueba prolongada queda fuera del alcance de esta etapa. El fallo anterior queda registrado en P-10.
5. Se estimó fs con generador en los cuatro presets y se midió caudal SPI entre 1 y 20 MHz; faltan la tolerancia del generador y la medición del reloj real. La prueba prolongada y la caracterización de la máxima velocidad estable quedan fuera del alcance de esta etapa.
