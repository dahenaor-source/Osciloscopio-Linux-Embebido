# 01. Descripción general

## 1. Objetivo

Construir un sistema híbrido **microprocesador + microcontrolador** que capture **dos señales analógicas** en ventanas de adquisición, como un osciloscopio, y las muestre en un **dashboard web** servido por el SBC a través de la red.

El proyecto explora las interfaces de entrada/salida: ADC del microcontrolador, **SPI** entre las dos placas y red/HTTP en el SBC.

## 2. Arquitectura

| Elemento | Hardware | Papel |
|---|---|---|
| Adquisición | **ESP32-C6-DevKitC-1** | Muestrea 2 canales con el ADC1 en modo continuo (DMA), calibra a milivoltios y entrega la ventana por SPI. Es **esclavo** SPI |
| Procesamiento y red | **BeaglePlay** (SoC TI AM625, Linux) | **Maestro** SPI. Ejecuta el backend, pide capturas, cambia el preset y sirve el dashboard por HTTP |
| Interfaz | Navegador (cualquier equipo de la red) | Dashboard con gráfica, estadísticas y controles |

```mermaid
flowchart LR
    G["Señales analógicas<br/>(generador o potenciómetro)"] -->|"GPIO0 = CH A<br/>GPIO1 = CH B"| ADC
    subgraph ESP["ESP32-C6 (firmware)"]
        ADC["ADC1 continuo<br/>+ DMA"] --> CAL["Tabla de calibración<br/>código a mV"] --> BUF["Buffers de captura<br/>A y B"] --> SLV["SPI2 esclavo"]
    end
    SLV <-->|"SPI modo 0<br/>SCK, MISO, MOSI, CS<br/>+ línea INT"| DRV
    subgraph SBC["BeaglePlay (Linux)"]
        DRV["spi_driver.py<br/>(spidev)"] --> API["Backend FastAPI<br/>puerto 8000"]
    end
    API <-->|"HTTP<br/>/api/scope/frame<br/>/api/scope/config"| WEB["Navegador<br/>Dashboard (Chart.js)"]
```

Conexión física: ver [03-hardware-y-conexiones.md](03-hardware-y-conexiones.md).

## 3. Flujo de una captura

El maestro **inicia** cada captura (el esclavo SPI no puede enviar por iniciativa propia). El esclavo captura una ventana, la deja lista y el maestro la lee.

```mermaid
sequenceDiagram
    participant N as Navegador
    participant B as Backend (BeaglePlay)
    participant E as ESP32-C6
    N->>B: GET /api/scope/frame
    B->>E: NOP (lee cabecera: fs, window, seq)
    B->>E: ARM
    Note over E: el ADC muestrea durante window/fs segundos
    Note over B: espera sin tráfico SPI (90 % de la ventana)
    loop hasta estado READY
        B->>E: NOP (lee cabecera)
        E-->>B: ARMED o READY
    end
    B->>E: NOP y lectura de 16 + 4 x window bytes
    E-->>B: cabecera + muestras A y B en mV
    B-->>N: JSON con seq, fs_hz, window, flags, a, b
```

Cambio de frecuencia de muestreo: el dashboard envía `POST /api/scope/config` con `{"rate_idx": N}` y el backend manda `SET_RATE` por SPI.

## 4. Máquina de estados del ESP32-C6

```mermaid
stateDiagram-v2
    [*] --> IDLE
    IDLE --> ARMED: ARM
    READY --> ARMED: ARM
    ARMED --> READY: ventana completa
    ARMED --> IDLE: RESET
    ARMED --> ERROR: fallo o timeout del ADC
    READY --> IDLE: SET_RATE, SET_WINDOW, SET_TESTMODE, RESET
    ERROR --> IDLE: SET_RATE, SET_WINDOW, SET_TESTMODE, RESET
```

Los comandos de configuración (`SET_RATE`, `SET_WINDOW`, `SET_TESTMODE`) durante `ARMED` abortan la captura y llevan el estado a `IDLE`; `ARM` con una captura en curso se rechaza (flag `comando rechazado`).

## 5. Tareas del firmware

```mermaid
flowchart TD
    A["spi_task: prepare_tx<br/>cabecera y, si READY, datos"] --> C["spi_slave_transmit<br/>espera la transacción del maestro"]
    C --> D["handle_command<br/>decodifica y ejecuta"]
    D -->|"comando ARM"| E["notifica a capture_task"]
    D --> A
    E --> F["capture_task: capture_run<br/>o patrón de prueba"]
    F --> G["estado READY (seq + 1)<br/>o ERROR"]
```

Detalle en [05-firmware-esp32c6.md](05-firmware-esp32c6.md).

## 6. Decisiones de diseño

| Decisión | Motivo |
|---|---|
| SBC = **BeaglePlay** en el socket **mikroBUS**, SBC maestro | Definido por el equipo; el maestro controla el ritmo de lectura |
| **ESP-IDF 5.4.1** (C) en el ESP32-C6 | Soporte completo del ADC continuo con DMA y del SPI esclavo con DMA |
| **ADC1 continuo** con temporización por hardware | Garantiza frecuencia de muestreo constante, sin depender del software |
| Captura **bajo demanda** (`ARM`) | El dashboard refresca a ≤ 1 Hz, no se necesita transmitir en continuo |
| Cabecera de estado en **toda** respuesta | El esclavo prepara la respuesta antes de la transacción; el efecto de un comando se ve en la transacción siguiente |
| Línea **INT** (handshake) | Permite al maestro saber cuándo el esclavo está listo (hoy el backend usa pausas y reintentos) |
| Muestras en **mV calibrados** (protocolo v2) | El ADC sin calibrar leía hasta 0,7 V de menos (ver [08](08-calibracion-adc.md)) |
| Backend **FastAPI** que también sirve el dashboard | Un solo origen y puerto, sin problemas de CORS (el enunciado pide apache o nginx: hay un `nginx.conf` sin probar, ver [13](13-pendientes-y-riesgos.md)) |
| Chart.js **local** (`dashboard/vendor`) | La BeaglePlay puede no tener Internet |

## 7. Alcance actual

Implementado y probado en hardware: enlace SPI a 2 MHz, modo de prueba, cambio de presets, captura real del ADC, dashboard con API real, calibración de eFuse (compila; pendiente medirla). Pendiente: servidor apache/nginx, verificación de frecuencia de muestreo con instrumentos, 40 kSa/s estable, tasa máxima de SPI, video y declaraciones. Ver [10](10-matriz-de-verificacion.md) y [13](13-pendientes-y-riesgos.md).
