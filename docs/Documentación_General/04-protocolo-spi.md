# 04. Protocolo SPI (versión 2)

El SBC es el **maestro** y la ESP32-C6 el **esclavo**. Modo SPI 0, 8 bits, MSB primero. Los enteros multibyte van en **little-endian**.

## Principio de funcionamiento

Cada transacción es **full-duplex**: el maestro envía un **comando de 4 bytes** (seguido de bytes de relleno) y, a la vez, recibe una **respuesta** que el esclavo ya tenía preparada.

- La respuesta siempre empieza con una **cabecera de 16 bytes** con el estado del esclavo.
- Si hay una captura lista (`state = READY`), tras la cabecera vienen los datos.
- Un comando se ejecuta **al terminar** la transacción; su efecto se ve en la cabecera de la **siguiente**.
- El maestro puede leer menos bytes de los disponibles (por ejemplo, 16 para consultar solo el estado).

## Comando (maestro → esclavo)

| Byte | Contenido |
|---|---|
| 0 | `cmd` |
| 1 | `arg` (byte bajo) |
| 2 | `arg` (byte alto) |
| 3 | checksum = `cmd ^ arg_lo ^ arg_hi ^ 0x5A` |

| `cmd` | Nombre | `arg` | Efecto |
|---|---|---|---|
| 0x00 | `NOP` | 0 | Solo consulta de estado/datos |
| 0x01 | `SET_RATE` | índice 0..3 | Selecciona la frecuencia de muestreo |
| 0x02 | `SET_WINDOW` | 64..1020 | Muestras por canal por ventana |
| 0x03 | `ARM` | 0 | Inicia una captura única |
| 0x04 | `RESET` | 0 | Aborta la captura y vuelve a `IDLE` |
| 0x05 | `SET_TESTMODE` | 0 o 1 | 1 = datos sintéticos (rampas) para probar el SPI sin ADC. Canal A = `i`, canal B = `4095 − i` (se interpretan como mV; no pasan por la calibración) |

Presets de `SET_RATE`:

| Índice | Muestreo por canal | Conversión del ADC (total) | Notas |
|---|---|---|---|
| 0 | 100 Sa/s | 2 kSa/s | Promedia 10 conversiones por muestra (el ADC no baja de 611 Sa/s) |
| 1 | 1.000 Sa/s | 2 kSa/s | — |
| 2 | 10.000 Sa/s | 20 kSa/s | — |
| 3 | 40.000 Sa/s | 80 kSa/s | Preset de bonificación; límite del ADC 83,3 kSa/s |

`SET_RATE`, `SET_WINDOW` y `SET_TESTMODE` descartan la captura anterior (estado → `IDLE`). **Desde el firmware de la rama `Osc-V2` ya no se rechazan durante una captura (`ARMED`)**: la abortan y aplican el cambio (antes se rechazaban con el flag `comando rechazado`). `ARM` sigue rechazándose si ya hay una captura en curso, y `RESET` aborta la captura. Un valor inválido (preset fuera de 0..3, ventana fuera de 64..1020, modo de prueba distinto de 0 o 1) se rechaza siempre.

## Respuesta (esclavo → maestro)

### Cabecera (16 bytes)

| Byte | Campo | Descripción |
|---|---|---|
| 0 | `magic` | Siempre `0xE5` |
| 1 | `state` | 0 `IDLE`, 1 `ARMED` (capturando), 2 `READY`, 3 `ERROR` |
| 2 | `flags` | bit0 desbordamiento del ADC, bit1 último comando rechazado, bit2 checksum de comando inválido, bit3 modo de prueba, bit4 ADC sin calibrar (sin datos de eFuse; los mV son una recta aproximada) |
| 3 | `rate_idx` | Preset actual |
| 4–5 | `window` | Muestras por canal |
| 6–7 | `seq` | Contador de capturas completadas |
| 8–11 | `fs_hz` | Frecuencia de muestreo por canal (Hz) |
| 12 | `n_channels` | 2 |
| 13 | `bits` | 12 |
| 14 | `fw_version` | Versión del protocolo (**2**) |
| 15 | `checksum` | XOR de los bytes 0..14 |

### Datos (solo si `state = READY`)

Formato plano: primero las `window` muestras del canal A y luego las del canal B, cada una como `uint16` little-endian en **milivoltios calibrados** (0..~3100). El ADC es de 12 bits; la conversión a mV la hace el ESP32 con una tabla de 4096 entradas por canal, construida con la calibración curve-fitting de la eFuse (sin coste de CPU en la captura).

Tamaño total = `16 + 4 × window` bytes (máximo 4096). Voltaje: `V = valor / 1000`. **Cambio v1 → v2:** en v1 las muestras eran el código crudo del ADC (0..4095).

> Las dos señales se convierten alternadas por el mismo ADC, de modo que el canal B va retrasado medio periodo de muestreo del ADC respecto al A (por ejemplo, 12,5 µs en el preset de 40 kSa/s).

## Secuencia de uso

```
1. SET_RATE idx         ──► (solo al cambiar la frecuencia)
2. SET_WINDOW n         ──► (solo al cambiar la ventana)
3. ARM
4. Consultar estado (NOP, leer 16 bytes) hasta state == READY
5. NOP leyendo 16 + 4·n bytes  ──► cabecera + datos
6. Repetir desde 3 en cada refresco del dashboard
```

Duración de una captura = `window / fs`. Ejemplos con 256 muestras: 2,56 s a 100 Sa/s, 256 ms a 1 kSa/s, 25,6 ms a 10 kSa/s, 6,4 ms a 40 kSa/s. Para refrescar el dashboard entre 1 y 4 s a 100 Sa/s, la ventana no debería superar unas 400 muestras.

Implementación de referencia de esta secuencia con reintentos: `capture_frame()` en [`beagleplay/backend/spi_driver.py`](../../beagleplay/backend/spi_driver.py) ([06-backend-beagleplay.md](06-backend-beagleplay.md)).

## Temporización y handshake

- La línea **INT** (GPIO3 del uC) está en alto mientras el esclavo tiene una transacción preparada, y en bajo mientras se re-arma tras cada transacción.
- Si el maestro no usa INT, debe esperar **≥ 2 ms** entre transacciones.
- Si el maestro inicia una transacción cuando el esclavo no está listo, la lectura no será válida: la cabecera fallará la verificación de `magic`/`checksum`. El maestro debe reintentar.

## Comportamiento observado en hardware

En las pruebas con la BeaglePlay real (2026-10-30, ver [09-pruebas-y-resultados.md](09-pruebas-y-resultados.md)) **cerca de la mitad de las transacciones llegan inválidas** y la causa raíz no está identificada. Tres formas de respuesta inválida:

| Respuesta | Interpretación |
|---|---|
| Todo `0xFF` | El esclavo no tenía la transacción preparada: **no recibió nada**, el comando no se ejecutó |
| `0xFF…` seguido de bytes de cabecera desplazados | El esclavo se armó a mitad de la transacción; recibió un comando corrupto y lo descartó (flag bit2 en la respuesta siguiente) |
| Cabecera válida con datos nuevos | Transacción correcta |

Reglas que el maestro debe seguir:

1. **Validar siempre** `magic = 0xE5` y el checksum de la cabecera antes de usar nada.
2. **Reintentar** la misma transacción hasta recibir una cabecera válida. Es seguro porque un comando corrupto se descarta por checksum.
3. **Comprobar `seq`** al capturar: si el `ARM` se perdió, el esclavo sigue en `READY` con la captura *anterior*; el maestro debe detectar que `seq` no avanzó y reenviar `ARM`.
4. **Evitar tráfico SPI durante el muestreo** (mitigación aplicada, hipótesis no confirmada): en una prueba apareció ruido periódico en el canal A que podría deberse al tráfico SPI ([09](09-pruebas-y-resultados.md), P-08 y P-09). El backend espera el 90 % de la ventana antes de consultar.
5. El flag bit2 (checksum de comando) queda pegado en las cabeceras siguientes a una transacción corrupta; el backend lo enmascara si ya reintentó con éxito y expone `retries` en su lugar.

⚠️ Riesgo conocido: el checksum de comando es de 8 bits; un comando corrupto podría pasarlo por azar (≈ 1/256) y ejecutarse. Por eso el backend verifica `rate_idx` tras `SET_RATE` y `seq` tras `ARM`.

## Historial de versiones

| Versión | Cambio |
|---|---|
| 1 | Primera versión. Las muestras eran el código crudo del ADC (0..4095) |
| 2 | Las muestras son **milivoltios calibrados**. Nuevo flag bit4 `ADC sin calibrar`. `fw_version = 2` en la cabecera |

## Referencia

Implementación en el esclavo: [`firmware/components/scope_proto`](../../firmware/components/scope_proto) y [`firmware/main/scope_link.c`](../../firmware/main/scope_link.c). Maestro de ejemplo: [`tools/spi_master_test.py`](../../tools/spi_master_test.py).
