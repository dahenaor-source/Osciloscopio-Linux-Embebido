import time
import math
from typing import List
from fastapi import FastAPI
from pydantic import BaseModel
from fastapi.middleware.cors import CORSMiddleware

app = FastAPI()

# Permitir peticiones desde el dashboard web (CORS)
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

# MODO DE TRABAJO: True = Datos simulados | False = Lectura real por SPI
USE_MOCK = True

# Estado global del osciloscopio
seq_counter = 0
current_rate_idx = 1
RATES_HZ = [100, 1000, 10000, 40000] # Frecuencias asociadas a los presets (0..3)

class ConfigPayload(BaseModel):
    rate_idx: int

def generar_frame_simulado(window: int = 256):
    """Genera 256 muestras de 12 bits (0 a 4095) para el Canal A y B."""
    global seq_counter
    seq_counter += 1
    t_now = time.time()
    
    a_samples: List[int] = []
    b_samples: List[int] = []
    
    fs = RATES_HZ[current_rate_idx]
    
    for i in range(window):
        t = t_now + (i / fs)
        # Senoidal en Canal A (alrededor de 2048, amplitud ~1500)
        val_a = int(2048 + 1500 * math.sin(2 * math.pi * 5 * t))
        # Cuadrada/Cos en Canal B
        val_b = int(2048 + 1200 * math.cos(2 * math.pi * 2 * t))
        
        # Limitar valores entre 0 y 4095 (12 bits)
        a_samples.append(max(0, min(4095, val_a)))
        b_samples.append(max(0, min(4095, val_b)))
        
    return {
        "seq": seq_counter,
        "rate_idx": current_rate_idx,
        "fs_hz": fs,
        "window": window,
        "flags": 0,
        "a": a_samples,
        "b": b_samples
    }

@app.get("/api/scope/frame")
def get_frame():
    """Entrega la última ventana capturada según el contrato de la API."""
    if USE_MOCK:
        return generar_frame_simulado()
    else:
        # TODO: Implementar lectura real de 256 muestras desde el SPI
        pass

@app.post("/api/scope/config")
def set_config(config: ConfigPayload):
    """Ajusta el preset de frecuencia de muestreo."""
    global current_rate_idx
    if 0 <= config.rate_idx <= 3:
        current_rate_idx = config.rate_idx
        # Si USE_MOCK es False, aquí se enviaría la instrucción SET_RATE por SPI al ESP32
        return {"status": "ok", "rate_idx": current_rate_idx}
    return {"error": "rate_idx inválido"}, 400