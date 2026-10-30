# Backend - Osciloscopio BeaglePlay

Servidor centralizado en Python con **FastAPI** para la BeaglePlay. Encargado de exponer la API REST para la transmisión de muestras de la señal del osciloscopio, gestionar la configuración de la frecuencia de muestreo y servir los archivos estáticos del dashboard web.

---

## 🛠️ Arquitectura de Software

El servidor unifica el acceso web y la transmisión de datos bajo un mismo origen (puerto `8000`), evitando bloqueos por políticas CORS y simplificando el despliegue en Linux Embebido.

```
Osciloscopio-Linux-Embebido/
├── beagleplay/
│   └── backend/
│       ├── main.py        # Servidor principal FastAPI + Lógica de simulación y API
│       └── README.md      # Documentación técnica del módulo
└── dashboard/             # Frontend estático (HTML, CSS, JS, Chart.js)
```

---

## 🛰️ Contrato de API Implementado

### 1. Obtener Ventana de Muestras
* **Ruta:** `GET /api/scope/frame`
* **Descripción:** Devuelve las últimas 256 muestras de 12 bits (0–4095) para los Canales A y B.
* **Respuesta JSON (`200 OK`):**
```json
{
  "seq": 5,
  "rate_idx": 1,
  "fs_hz": 1000,
  "window": 256,
  "flags": 0,
  "a": [2048, 2051, 2055],
  "b": [1020, 1018, 1015]
}
```

### 2. Configurar Frecuencia de Muestreo
* **Ruta:** `POST /api/scope/config`
* **Cuerpo de la Petición (JSON):**
```json
{
  "rate_idx": 1
}
```
* **Respuesta JSON (`200 OK`):**
```json
{
  "status": "ok",
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

El backend cuenta con una bandera global en `main.py` para alternar el origen de los datos:

* **`USE_MOCK = True` (Modo Simulación):** Genera internamente dos señales sintéticas en matemática discreta (Senoidal a 5 Hz en Canal A y Coseno a 2 Hz en Canal B). No requiere hardware externo de captura.
* **`USE_MOCK = False` (Modo SPI Real):** Habilita la comunicación por bus SPI (`spidev`) para recibir los paquetes directamente desde la ESP32-C6.

---

## 🚀 Requisitos e Instalación

### Dependencias en la BeaglePlay
```bash
sudo apt update
sudo apt install python3-pip -y
pip3 install fastapi uvicorn pydantic spidev
```

---

## 💻 Ejecución del Servidor

Desde la carpeta `beagleplay/backend`:

```bash
python3 -m uvicorn main:app --host 0.0.0.0 --port 8000
```

### Acceso a la Interfaz y Documentación
* **Dashboard Web:** `http://<IP_BEAGLEPLAY>:8000/`
* **Swagger UI (Pruebas API):** `http://<IP_BEAGLEPLAY>:8000/docs`