# Pinout y conexiones

Conexión entre el puerto **mikroBUS de la BeaglePlay** (SPI maestro) y la **ESP32-C6-DevKitC-1** (SPI esclavo).

## Tabla de conexiones

| Señal | mikroBUS (pin) | BeaglePlay | ESP32-C6-DevKitC-1 | Dirección |
|---|---|---|---|---|
| SCK | 4 | SPI CLK | **GPIO6** (FSPICLK) | SBC → uC |
| MISO | 5 | SPI POCI | **GPIO2** (FSPIQ) | uC → SBC |
| MOSI | 6 | SPI PICO | **GPIO7** (FSPID) | SBC → uC |
| CS | 3 | SPI CS | **GPIO10** | SBC → uC |
| INT (handshake) | 11 | GPIO de entrada | **GPIO3** | uC → SBC |
| RST (opcional) | 2 | GPIO de salida | **EN** (reset) | SBC → uC |
| GND | 8 (o 16) | GND | **GND** | común |
| 3,3 V (mikroBUS pin 7) | 7 | 3,3 V | **no conectar** | — |

Pines mikroBUS según el estándar (vista del conector): fila izquierda 1–8 = AN, RST, CS, SCK, MISO, MOSI, 3,3V, GND; fila derecha 9–16 = PWM, INT, RX, TX, SCL, SDA, 5V, GND.

> Todas las señales son de **3,3 V**. No conectar el pin 5V del mikroBUS a ningún GPIO. La ESP32-C6 se alimenta por su puerto USB; solo se comparte GND con la BeaglePlay.

## Entradas analógicas (ADC1)

| Canal | Pin ESP32-C6 | Uso |
|---|---|---|
| CH A | **GPIO0** (ADC1_CH0) | Señal 1 del generador |
| CH B | **GPIO1** (ADC1_CH1) | Señal 2 del generador |

- Rango de entrada: **0 a ~3,1 V** (atenuación 12 dB). El ADC no mide voltajes negativos y puede dañarse por encima de 3,3 V.
- Configurar el generador de señales con **offset positivo** (por ejemplo, seno de 1,5 V de amplitud pico y 1,5 V de offset) y verificar con el osciloscopio/analizador antes de conectar.
- Recomendado (opcional): resistencia de 1 kΩ en serie en cada entrada para limitar corriente.
- Unir la tierra del generador con GND de la ESP32-C6.

## Por qué estos pines

- **GPIO6, GPIO7 y GPIO2** son los pines nativos (IO_MUX) de SPI2 en el ESP32-C6, necesarios para alcanzar velocidades altas como esclavo.
- El CS nativo es el GPIO16, que es el TX de la UART de consola/flasheo de la DevKitC-1; por eso el CS usa GPIO10 (matriz de GPIO).
- GPIO0 y GPIO1 son canales del ADC1 y no se solapan con el SPI. Se evitan los pines de arranque (GPIO4, 5, 8, 9, 15).

## En la BeaglePlay (por definir por el equipo del SBC)

- El puerto mikroBUS está gestionado por defecto por el driver `mikrobus`; para usar el SPI como `/dev/spidevX.Y` hay que deshabilitar ese driver o usar un overlay. **La ruta del dispositivo aún no está determinada.** Se obtiene con `ls /dev/spidev*` una vez configurado.
- Parámetros del maestro: **modo 0** (CPOL=0, CPHA=0), 8 bits por palabra, MSB primero.
- Tamaño máximo de transacción: 4096 bytes, igual al `bufsiz` por defecto de `spidev`.
- La línea INT (handshake) puede leerse con `libgpiod`. Si no se usa, esperar ≥ 2 ms entre transacciones.
