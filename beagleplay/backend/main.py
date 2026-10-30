import asyncio
import math
import time
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
import spidev

app = FastAPI()

# True = Simulación activa | False = Lectura real por SPI
USE_MOCK = True

if not USE_MOCK:
    try:
        spi = spidev.SpiDev()
        spi.open(0, 0)
        spi.max_speed_hz = 1000000
        spi.mode = 0
    except Exception as e:
        print(f"Error al abrir SPI: {e}")

def leer_datos():
    if USE_MOCK:
        t = time.time()
        ch_a = int((math.sin(2 * math.pi * 5 * t) + 1) * 2047.5)  # Senoidal 5 Hz
        ch_b = int((math.cos(2 * math.pi * 2 * t) + 1) * 2047.5)  # Senoidal 2 Hz
        return {"ch_a": ch_a, "ch_b": ch_b}
    else:
        bytes_recibidos = spi.xfer2([0x00, 0x00, 0x00, 0x00])
        ch_a = (bytes_recibidos[0] << 8) | bytes_recibidos[1]
        ch_b = (bytes_recibidos[2] << 8) | bytes_recibidos[3]
        return {"ch_a": ch_a, "ch_b": ch_b}

@app.websocket("/ws/data")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    print("Cliente web conectado para streaming de datos.")
    try:
        while True:
            datos = leer_datos()
            await websocket.send_json(datos)
            await asyncio.sleep(0.01)  # Muestreo cada 10 ms
    except WebSocketDisconnect:
        print("Cliente web desconectado.")