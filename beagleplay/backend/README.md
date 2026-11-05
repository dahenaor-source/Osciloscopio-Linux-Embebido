# Backend - Osciloscopio BeaglePlay

Servidor centralizado en Python con **FastAPI** para la BeaglePlay. Encargado de exponer la API REST para la transmisión de muestras de la señal del osciloscopio, gestionar la configuración de la frecuencia de muestreo, ejecutar la comunicación SPI con la ESP32-C6 y servir los archivos estáticos del dashboard web.

---

## 🛠️ Arquitectura de Software

FastAPI expone los endpoints en el puerto `8000` (normalmente ligado a `127.0.0.1`). **nginx sirve la interfaz en el puerto `80` y reenvía `/api/` al backend**. El navegador accede al puerto 80 bajo el mismo origen.

```
beagleplay/backend/
├── main.py        # Servidor principal FastAPI, rutas API y servidor estático
├── spi_driver.py  # Driver del bus SPI (cabeceras de 16 B, checksum XOR, little-endian; reintentos y candado)
└── README.md      # Documentación técnica del módulo backend
```

---

## 🔌 Driver SPI (`spi_driver.py`)

El módulo `SPIDriver` implementa la comunicación Full-Duplex en **Modo SPI 0** hacia la ESP32-C6 utilizando el formato de comandos heredado de v1 y el **protocolo de respuesta v2** (muestras en milivoltios). La versión recibida debe comprobarse en la cabecera:

* **Trama de comando (4 bytes):** Envío de instrucciones (`NOP`, `SET_RATE`, `ARM`, etc.) con cálculo de checksum `cmd ^ arg_lo ^ arg_hi ^ 0x5A`.
* **Validación de respuesta:** Verificación del byte mágico (`0xE5`) y checksum de la cabecera de 16 bytes.
* **Desempaquetado de datos:** Conversión de bytes little-endian a muestras `uint16` para los Canales A y B. Desde el protocolo v2 son **milivoltios calibrados** por el ESP32-C6 (antes, código crudo de 12 bits).
* **Tiempos de guarda:** Control de pausas (≥ 2 ms) entre transacciones SPI para estabilidad de enlace.

---

## 🛰️ Contrato de API Implementado

### 1. Obtener Ventana de Muestras
* **Ruta:** `GET /api/scope/frame`
* **Descripción:** Captura y devuelve una ventana (256 muestras por defecto) en mV para los Canales A y B, más `retries` (transacciones SPI repetidas).
* **Respuesta JSON (`200 OK`):**
```json
{
  "seq": 5,
  "rate_idx": 1,
  "fs_hz": 1000,
  "window": 256,
  "flags": 0,
  "retries": 0,
  "a": [1650, 1655, 1661],
  "b": [820, 818, 815]
}
```

### 2. Configurar Frecuencia de Muestreo
* **Ruta:** `POST /api/scope/config`
* **Cuerpo JSON:**
```json
{
  "rate_idx": 1
}
```

* **Tabla de Presets (`rate_idx`):**

| `rate_idx` | Frecuencia de Muestreo (`fs_hz`) |
| :---: | :---: |
| `0` | 100 Sa/s |
| `1` | 1.000 Sa/s (1 kSa/s) |
| `2` | 10.000 Sa/s (10 kSa/s) |
| `3` | 40.000 Sa/s (40 kSa/s) |

---

## ⚙️ Modos de Operación (`USE_MOCK`)

En `main.py` se controla el origen de los datos mediante la bandera global:

* **`USE_MOCK = True` (Modo Simulación):** Genera matemático-discretamente ondas senoidales sintéticas para pruebas de interfaz sin necesidad del hardware de captura.
* **`USE_MOCK = False` (Modo SPI Real):** Inicializa `/dev/spidev0.0` mediante `spi_driver.py` para la adquisición física de datos desde la ESP32-C6.

---

## 💻 Ejecución del Servidor

**Instalación del arranque automático (pendiente de validación completa en la placa).** El instalador crea e inicia una unidad systemd. Tras instalar, verificar `systemctl is-active`, reiniciar BeaglePlay y comprobar la API real; no dar el sistema por validado únicamente por ejecutar el script:

```bash
sudo bash ~/Osciloscopio-Linux-Embebido/beagleplay/instalar_servicio.sh
```

Registro: `sudo journalctl -u osciloscopio-backend -f`. Reiniciar tras un `git pull`: `sudo systemctl restart osciloscopio-backend`.

**A mano (depuración).** Antes hay que parar el servicio (`sudo systemctl stop osciloscopio-backend`) para liberar el puerto 8000:

```bash
cd beagleplay/backend
SPI_SPEED_HZ=2000000 sudo -E python3 -m uvicorn main:app --host 127.0.0.1 --port 8000
```

`SPI_SPEED_HZ` (por defecto 2000000) fija la velocidad del SPI; 2 MHz es la que funcionó y a 100 kHz falla más. Reintentos, tiempos, servicio systemd y diagnóstico de errores: [`docs/Documentación_General/06-backend-beagleplay.md`](../../docs/Documentación_General/06-backend-beagleplay.md).

### Accesos
* **Dashboard Web (nginx):** `http://<IP_BEAGLEPLAY>/`. Con `--host 0.0.0.0` también `http://<IP_BEAGLEPLAY>:8000/`.
* **Documentación Interactiva (Swagger UI):** `http://127.0.0.1:8000/docs` desde la BeaglePlay, o `http://<IP_BEAGLEPLAY>:8000/docs` si se inicia manualmente con `--host 0.0.0.0`.