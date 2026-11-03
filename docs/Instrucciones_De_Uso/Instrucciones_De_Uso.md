# Instrucciones de uso: arma tu propio osciloscopio (BeaglePlay + ESP32-C6)

Esta guía te lleva **desde cero** hasta ver en tu navegador la gráfica de dos señales eléctricas medidas por el sistema. No necesitas saber de osciloscopios ni de electrónica. No tienes que modificar ningún código ni instalar entornos de programación pesados: **copias el repositorio, flasheas el ESP32-C6, montas los cables y enciendes todo**.

Tiempo estimado: 1 a 2 horas la primera vez.

> **Estado del proyecto (2026-11-03).** El sistema funciona de extremo a extremo con 100, 1.000 y 10.000 muestras por segundo. Hay limitaciones conocidas que **no son errores tuyos**: el preset de 40.000 Sa/s falla, cerca de la mitad de las lecturas SPI se pierden y el software las reintenta, y la calibración del ADC de la versión actual del firmware (v2) todavía no se ha medido con instrumentos. Están explicadas en la sección 11.
>
> Los pasos marcados con ⚠️ dependen de detalles de la BeaglePlay del equipo (por ejemplo la imagen de Linux del profesor) que **no se registraron**. Se explican de forma genérica y se indica cómo comprobarlos.

## Índice

0. [¿Qué vas a construir?](#0-qué-vas-a-construir)
1. [Lista de materiales](#1-lista-de-materiales)
2. [Reglas de seguridad (léelas antes de conectar nada)](#2-reglas-de-seguridad)
3. [Paso 1: descargar el repositorio](#3-paso-1-descargar-el-repositorio)
4. [Paso 2: flashear el ESP32-C6](#4-paso-2-flashear-el-esp32-c6)
5. [Paso 3: preparar la BeaglePlay](#5-paso-3-preparar-la-beagleplay)
6. [Paso 4: montaje físico](#6-paso-4-montaje-físico)
7. [Paso 5: comprobar que las placas se hablan](#7-paso-5-comprobar-que-las-placas-se-hablan)
8. [Paso 6: encender el sistema y abrir el dashboard](#8-paso-6-encender-el-sistema-y-abrir-el-dashboard)
9. [Cómo usar el dashboard](#9-cómo-usar-el-dashboard)
10. [Problemas comunes y soluciones](#10-problemas-comunes-y-soluciones)
11. [Limitaciones conocidas](#11-limitaciones-conocidas)
12. [Apagar y guardar](#12-apagar-y-guardar)
13. [Resumen de comandos](#13-resumen-de-comandos)

---

## 0. ¿Qué vas a construir?

Un **osciloscopio** es un instrumento que dibuja cómo cambia un **voltaje** (la «fuerza» de una señal eléctrica) a lo largo del tiempo. Este montaje mide **dos señales a la vez** (canal A y canal B) y las dibuja en una página web.

```
Señal (0 a 3,1 V) ─► ESP32-C6 ══SPI══► BeaglePlay ──red──► Navegador de tu PC
   (la mide)        (la convierte      (la guarda y         (la dibuja)
                     en números)        la publica)
```

![Vista general del montaje](img/vista-general-montaje.svg)

| Pieza | Qué hace, en palabras simples |
|---|---|
| **ESP32-C6** (microcontrolador) | Un computador diminuto con un «medidor de voltaje» (el ADC). Toma muchas mediciones por segundo |
| **BeaglePlay** (SBC, computador de placa única) | Un computador pequeño con Linux. Pide los datos al ESP32-C6 y los publica como una página web |
| **SPI** | El «cable de conversación» entre las dos placas (4 hilos de datos más tierra) |
| **Dashboard** | La página web con la gráfica, que ves desde tu PC |

### Glosario mínimo

| Palabra | Significa |
|---|---|
| **Voltaje (V)** | Lo que se mide. Aquí de 0 a unos 3,1 voltios |
| **Muestra** | Una medición de voltaje en un instante |
| **Sa/s** (muestras por segundo) | Cuántas mediciones se toman cada segundo. 1.000 Sa/s = una medición cada milésima de segundo |
| **Canal** | Una de las dos entradas de señal (A o B) |
| **GND (tierra)** | El «cero voltios» común. Todas las piezas deben compartirlo |
| **GPIO** | Un pin del microcontrolador (GPIO0 = pin número 0) |
| **Flashear** | Grabar el programa (firmware) en el microcontrolador |
| **Firmware** | El programa que corre dentro del ESP32-C6 |
| **mikroBUS** | El conector estándar de 16 pines de la BeaglePlay donde se conectan los cables SPI |
| **Potenciómetro** | Una perilla que entrega un voltaje ajustable; sirve de «señal de prueba» |

---

## 1. Lista de materiales

### Hardware

| # | Cantidad | Elemento | Notas |
|---|---|---|---|
| 1 | 1 | **ESP32-C6-DevKitC-1** | Debe ser exactamente este modelo (los pines están asignados para él) |
| 2 | 1 | **BeaglePlay** | Con la imagen de Linux que entregó el profesor (⚠️ el equipo no registró la versión). Debe traer habilitado `/dev/spidev0.0` |
| 3 | 2 | Cables **USB-C de datos** | Uno para cada placa. Los de «solo carga» no sirven para flashear ni para la red |
| 4 | 6 | Cables Dupont **macho-hembra** | Para unir el conector mikroBUS de la BeaglePlay (hembra: tiene orificios) con los pines del ESP32-C6. Uno es opcional |
| 5 | 1 | **Potenciómetro de 10 kΩ** | Señal de prueba (10 kΩ es el valor recomendado; el equipo no registró el que usó). Alternativa: un generador de señales (0 a 3,1 V, solo valores positivos) |
| 6 | 1 | Resistencia de **1 kΩ** | Protege la entrada analógica |
| 7 | 1 | Condensador de **100 nF** (cerámico) | Reduce el ruido. ⚠️ El equipo no registró el valor usado; 100 nF es el valor típico y recomendado |
| 8 | 1 | Protoboard (placa de pruebas) | Para el circuito del potenciómetro |
| 9 | 6 a 8 | Cables Dupont macho-macho | Para el circuito en la protoboard |
| 10 | 1 | Multímetro | **Opcional**, para comparar voltajes |

### Lo que necesitas en tu computador

| Elemento | Para qué |
|---|---|
| Windows, Linux o WSL (Ubuntu) | Para flashear y conectarte |
| **Python 3** | Para ejecutar la herramienta de flasheo (`esptool`) |
| **Git** (o descargar el ZIP desde GitHub) | Para copiar el repositorio |
| Un cliente **SSH** | Ya viene en Windows 10/11 (PowerShell) y en Linux |
| Un navegador web | Para ver el dashboard |
| Conexión a **Internet** | Para instalar `esptool` y, si faltan, los paquetes de la BeaglePlay |

### Lo que NO necesitas

- No necesitas ESP-IDF ni compilar nada: el firmware ya está compilado en `release/`.
- No necesitas generador de señales ni analizador lógico.
- No tienes que modificar el código.

---

## 2. Reglas de seguridad

1. **Voltaje máximo en cualquier pin: 3,3 V.** Más puede dañar el ESP32-C6. La entrada analógica trabaja de **0 a ~3,1 V y solo positivo** (nunca voltajes negativos).
2. **Entre las dos placas se unen únicamente GND y las señales SPI. NUNCA unas 3V3 ni 5V de una placa con la otra.** Cada placa se alimenta por su propio cable USB-C.
3. **Conecta los cables con todo apagado** (ambas placas sin USB). Revisa antes de encender.
4. **Guíate por los nombres impresos** junto a cada pin (CS, SCK, MISO, MOSI, GND). Los números de esta guía son de apoyo.
5. Toca las placas por los bordes y descárgate antes (toca un objeto metálico con toma de tierra): la electricidad estática las puede dañar.
6. No quites la alimentación de la BeaglePlay sin apagarla antes (ver sección 12): puede corromper su memoria.

---

## 3. Paso 1: descargar el repositorio

**Qué haces:** copiar los archivos del proyecto a tu PC.

Abre una terminal (en Windows: PowerShell; en Linux/WSL: Terminal) y escribe:

```bash
git clone https://github.com/dahenaor-source/Osciloscopio-Linux-Embebido.git
cd Osciloscopio-Linux-Embebido
```

**Sin Git:** en la página del repositorio pulsa **Code → Download ZIP**, descomprime y abre una terminal dentro de la carpeta resultante.

**Comprobación:** deben existir las carpetas `release`, `dashboard`, `beagleplay` y `docs`:

```bash
ls            # Linux/WSL
dir           # Windows PowerShell
```

---

## 4. Paso 2: flashear el ESP32-C6

**Qué haces:** grabar el programa `release/osciloscopio_c6_v2.bin` dentro del ESP32-C6. Esto se hace una sola vez (el programa queda guardado aunque lo desconectes).

### 4.1 Conecta la placa

1. Conecta el ESP32-C6 a tu PC con un cable USB-C de datos. La DevKitC-1 tiene **dos** conectores USB-C: usa el rotulado **USB** (el que usó el equipo). El rotulado UART también sirve.
2. Debe encenderse un LED en la placa.

### 4.2 Instala la herramienta de flasheo

| Sistema | Comando |
|---|---|
| **Windows** (instala Python desde python.org y marca «Add Python to PATH») | `py -m pip install esptool` |
| **Linux / WSL** | `python3 -m venv ~/venv-esp && . ~/venv-esp/bin/activate && pip install esptool` |

### 4.3 Averigua el puerto de la placa

| Sistema | Cómo |
|---|---|
| **Windows** | Administrador de dispositivos → «Puertos (COM y LPT)». Aparece, por ejemplo, `COM5`. Si no aparece, prueba otro cable |
| **Linux** | `ls /dev/ttyACM* /dev/ttyUSB*` (por ejemplo `/dev/ttyACM0`). Si dice «permiso denegado»: `sudo usermod -aG dialout $USER` y reinicia la sesión |
| **WSL** | WSL no ve el USB por defecto. Sigue la sección «Acceso al USB desde WSL (usbipd)» de [05-firmware-esp32c6.md](../Documentaci%C3%B3n_General/05-firmware-esp32c6.md); después el puerto es `/dev/ttyACM0` |

### 4.4 Flashea

Reemplaza `COM5` o `/dev/ttyACM0` por tu puerto. Ejecuta el comando **desde la carpeta del repositorio**:

```bash
# Windows (PowerShell)
py -m esptool --chip esp32c6 -p COM5 -b 460800 write_flash 0x0 release\osciloscopio_c6_v2.bin

# Linux / WSL
python3 -m esptool --chip esp32c6 -p /dev/ttyACM0 -b 460800 write_flash 0x0 release/osciloscopio_c6_v2.bin
```

**Resultado esperado:** una barra de progreso y al final `Hash of data verified.` y `Hard resetting via RTS pin...`.

**Si no conecta** (`Failed to connect` o `Timed out waiting for packet header`): mantén pulsado el botón **BOOT** de la placa, pulsa y suelta **RESET** (o **EN**), suelta **BOOT** y repite el comando.

**Opcional, comprobar que el archivo es el correcto:** su huella SHA-256 debe ser `1c51a29df9c500cb06603873d2eb6444d1340b4ed6fb0c01d0ccdb8b76a5ac9c`.

```bash
sha256sum release/osciloscopio_c6_v2.bin               # Linux / WSL
Get-FileHash release\osciloscopio_c6_v2.bin -Algorithm SHA256    # Windows
```

### 4.5 Comprueba que arrancó

Abre el monitor serie (muestra lo que la placa escribe):

```bash
py -m serial.tools.miniterm COM5 115200              # Windows
python3 -m serial.tools.miniterm /dev/ttyACM0 115200   # Linux / WSL
```

Pulsa el botón **RESET** de la placa. Debes ver un mensaje que incluye:

```
scope_link: SPI esclavo listo (SCLK=6 MOSI=7 MISO=2 CS=10 HS=3)
```

Con el firmware v2 aparecen además líneas como `capture: canal 0: calibracion curve-fitting (eFuse)` (una por canal). ⚠️ Esas líneas de calibración **no se han verificado en una placa real todavía**; si en su lugar aparece un nombre de error, el sistema funciona igual, pero el dashboard mostrará «ADC sin calibrar» y los voltajes serán aproximados.

Para salir del monitor: `Ctrl + ]`. Luego **desconecta el cable USB** para el montaje.

---

## 5. Paso 3: preparar la BeaglePlay

**Qué haces:** encender la BeaglePlay, conectarte a ella desde tu PC, copiarle el proyecto y comprobar que tiene lo necesario. **Aún no conectes los cables SPI.**

### 5.1 Imagen de Linux

La BeaglePlay debe tener la imagen de Linux que entregó el profesor en su almacenamiento (microSD o memoria interna). ⚠️ El equipo no registró cuál usó. Si tu placa ya inicia Linux, no tienes que hacer nada. Si tienes que grabarla en una microSD, usa un programa como **balenaEtcher** y la imagen que te dé el profesor.

### 5.2 Enciéndela y conéctate

1. Conecta la BeaglePlay a tu PC con un cable **USB-C** al puerto USB-C de la placa. Eso la alimenta y crea una red USB entre ambos.
2. Espera **1 a 2 minutos** a que arranque Linux.
3. Desde tu PC, conéctate por SSH. Por USB, la BeaglePlay tiene la dirección **192.168.7.2**:

```bash
ssh USUARIO@192.168.7.2
```

Reemplaza `USUARIO` por el usuario de tu imagen. El equipo usó `camilo`; ⚠️ en las imágenes oficiales de BeagleBoard.org el usuario es `debian` con contraseña `temppwd`. Pregunta al profesor si no lo sabes. Responde `yes` a la pregunta de confianza y escribe la contraseña (no se ve al escribir).

**Resultado esperado:** un mensaje de bienvenida y un prompt como `usuario@BeagleBone:~$`.

**Si no responde:** en Windows puede faltar el driver de red USB (RNDIS/CDC); revisa el Administrador de dispositivos. Alternativa: conecta la BeaglePlay por cable **Ethernet** a tu router y busca su IP.

### 5.3 Copia el repositorio a la BeaglePlay

**Opción A, desde la BeaglePlay (si tiene Internet):** en la sesión SSH:

```bash
git clone https://github.com/dahenaor-source/Osciloscopio-Linux-Embebido.git
```

**Opción B, desde tu PC (no requiere Internet en la BeaglePlay).** En una terminal de tu PC, **fuera** de la sesión SSH y desde la carpeta que contiene el repositorio:

```bash
scp -r Osciloscopio-Linux-Embebido USUARIO@192.168.7.2:~/
```

En ambos casos queda la carpeta `~/Osciloscopio-Linux-Embebido` en la BeaglePlay.

### 5.4 Comprueba el puerto SPI

En la sesión SSH de la BeaglePlay:

```bash
ls /dev/spidev*
```

**Resultado esperado:** aparece `/dev/spidev0.0`. El software usa exactamente ese nombre.

⚠️ Si no existe, tu imagen no trae habilitado el SPI del mikroBUS. En la imagen del equipo ya venía habilitado y **no hubo que configurar nada**, por eso este paso no está documentado. Consulta con el profesor y revisa la sección 4 de [12-guia-equipo-sbc.md](../Documentaci%C3%B3n_General/12-guia-equipo-sbc.md); no continúes hasta que exista.

### 5.5 Comprueba las dependencias de Python

El backend necesita `fastapi`, `uvicorn` y `spidev`, y se ejecuta con `sudo`, así que `sudo python3` debe encontrarlos. Pruébalo:

```bash
sudo python3 -c "import fastapi, uvicorn, spidev; print('Todo instalado')"
```

- Si escribe `Todo instalado`: sigue al paso 6.
- Si da `ModuleNotFoundError`, instálalos (necesitas Internet en la BeaglePlay):

```bash
sudo apt update
sudo apt install -y python3-pip python3-spidev python3-fastapi python3-uvicorn
sudo python3 -c "import fastapi, uvicorn, spidev; print('Todo instalado')"
```

⚠️ El equipo no registró cómo instaló estos paquetes originalmente; estos comandos son el método estándar de Debian y **no se han ejecutado en su placa**. Si `apt` no encuentra algún paquete, usa `sudo pip3 install -r ~/Osciloscopio-Linux-Embebido/beagleplay/backend/requirements.txt` (si pide `--break-system-packages`, añádelo).

### 5.6 Apaga la BeaglePlay para el montaje

```bash
sudo poweroff
```

Espera a que se apaguen los LED (unos 10 segundos) y desconecta el cable USB-C.

---

## 6. Paso 4: montaje físico

> **Todo desconectado de la corriente.** Ni la BeaglePlay ni el ESP32-C6 deben tener cable USB mientras conectas los cables.

### 6.1 Cables SPI: BeaglePlay (mikroBUS) ↔ ESP32-C6

Se usan **5 cables obligatorios** (SCK, MISO, MOSI, CS y GND) y **1 opcional** (INT; el software actual no lo usa, pero déjalo conectado si quieres el montaje completo).

| # | Señal | Pin en el mikroBUS de la BeaglePlay (nombre impreso) | Pin del ESP32-C6 (número impreso) | Color sugerido |
|---|---|---|---|---|
| 1 | **SCK** (reloj) | `SCK` (pin 4) | **6** | Amarillo |
| 2 | **MISO** (datos hacia la BeaglePlay) | `MISO` (pin 5) | **2** | Verde |
| 3 | **MOSI** (datos hacia el ESP32-C6) | `MOSI` (pin 6) | **7** | Azul |
| 4 | **CS** (selección de chip) | `CS` (pin 3) | **10** | Naranja |
| 5 | **GND** (tierra) | `GND` (pin 8, o pin 9 del otro lado) | **G** (cualquier pin GND) | Negro |
| 6 | INT (opcional) | `INT` (pin 15) | **3** | Blanco |

**No conectes:** `+3.3V` (pin 7), `+5V` (pin 10), `RST`, `AN`, `PWM`, `RX`, `TX`, `SCL`, `SDA`.

Cómo leer el conector mikroBUS de la BeaglePlay (vista desde arriba, con el logo «mikroBUS» hacia arriba): son dos filas de 8 orificios. Cada orificio tiene su nombre impreso:

| Fila izquierda (de arriba abajo) | | Fila derecha (de abajo arriba) |
|---|---|---|
| AN, RST, **CS**, **SCK**, **MISO**, **MOSI**, +3.3V, **GND** | | GND, +5V, SDA, SCL, TX, RX, **INT**, PWM |

![Conexión SPI entre la BeaglePlay y la ESP32-C6](../Documentaci%C3%B3n_General/img/conexion-spi.svg)

⚠️ La numeración 1 a 16 se confirmó con el equipo, pero **no se ha contrastado con el esquemático de la BeaglePlay**. Si el nombre impreso y el número de esta tabla no coinciden, **manda el nombre impreso**. Hay más detalle en [03-hardware-y-conexiones.md](../Documentaci%C3%B3n_General/03-hardware-y-conexiones.md).

**Pasos:**

1. Mira cada pin del ESP32-C6: el **número** está serigrafiado junto al pin (busca `6`, `2`, `7`, `10`, `3` y `G`).
2. Conecta el extremo **macho** de cada cable al orificio correspondiente del mikroBUS (según la tabla) y el extremo **hembra** al pin del ESP32-C6.
3. Haz un solo cable a la vez y márcalo en la tabla.
4. Revisa que ningún cable toque otro pin vecino (los pines de `MOSI`, `+3.3V` y `GND` están juntos).

### 6.2 Señal de prueba: potenciómetro (circuito en la protoboard)

El potenciómetro tiene **3 patas**: dos extremos y una central (el *cursor*). Al girar la perilla, el voltaje de la pata central sube o baja entre 0 V y 3,3 V.

![Circuito del potenciómetro](img/circuito-potenciometro.svg)

| # | De | A |
|---|---|---|
| 1 | Pata **extremo 1** del potenciómetro | Pin **3V3** del ESP32-C6 (este 3V3 solo alimenta el potenciómetro) |
| 2 | Pata **extremo 3** del potenciómetro | Pin **G** (GND) del ESP32-C6 |
| 3 | Pata **central** (cursor) | Un terminal de la resistencia de **1 kΩ** |
| 4 | Otro terminal de la resistencia de 1 kΩ | Pin **0** del ESP32-C6 (**GPIO0 = canal A**) |
| 5 | Una pata del **condensador de 100 nF** | El mismo punto del paso 4 (entre la resistencia y el pin 0) |
| 6 | La otra pata del condensador | GND |
| 7 | Pin **1** del ESP32-C6 (**GPIO1 = canal B**) | GND, si no usas un segundo potenciómetro (el canal B leerá 0 V) |

Para usar **dos señales**, repite los pasos 1 a 6 con un segundo potenciómetro y conéctalo al pin **1** (GPIO1). Si no tienes potenciómetro, puedes usar un generador de señales con salida de 0 a 3,1 V y **offset positivo**; su tierra va a GND del ESP32-C6.

**Importante:** las pruebas del equipo se hicieron con el condensador. Sin él, se vio ruido en el canal A (ver sección 10).

### 6.3 Revisión antes de encender

Marca cada punto:

- [ ] Hay **5 cables SPI** y cada uno va al nombre correcto (SCK, MISO, MOSI, CS y GND).
- [ ] **Ningún** cable va a `+3.3V` ni a `+5V` del mikroBUS.
- [ ] El potenciómetro tiene un extremo en 3V3 y el otro en GND del ESP32-C6, y el cursor entra a GPIO0 por la resistencia.
- [ ] Ningún cable toca un pin vecino y no hay cables sueltos.
- [ ] Las dos placas siguen **sin** cable USB.

---

## 7. Paso 5: comprobar que las placas se hablan

Antes de usar el dashboard, comprueba solo el enlace SPI.

1. **Primero el ESP32-C6:** conecta su cable USB-C a una fuente de energía (tu PC o un cargador USB). Espera 2 segundos.
2. **Luego la BeaglePlay:** conecta su cable USB-C a tu PC y espera 1 a 2 minutos.
3. Entra por SSH (`ssh USUARIO@192.168.7.2`) y ejecuta:

```bash
cd ~/Osciloscopio-Linux-Embebido/beagleplay/backend
sudo python3 prueba_directa_spi.py
```

El script envía hasta 5 veces una consulta de estado al ESP32-C6.

**Resultado esperado** (ejemplo real del equipo):

```
PRUEBA 1
RX: ['FF', 'FF', 'FF', ...]
Sin respuesta valida del ESP32-C6

PRUEBA 2
RX: ['E5', '00', '00', '01', '00', '01', '00', '00', 'E8', '03', ...]
SPI OK: cabecera valida
```

- **Es normal** que alguna lectura salga toda en `FF`: cerca de la mitad de las lecturas SPI se pierden (problema conocido). Lo importante es que **alguna** de las 5 diga `SPI OK: cabecera valida`.
- Si las 5 fallan: repite el comando 2 o 3 veces. Si siguen fallando, apaga todo y revisa los cables (sección 10).

**Segunda prueba, con datos de ejemplo conocidos (opcional).** Pone al ESP32-C6 en «modo de prueba» y lee una captura de rampas:

```bash
sudo python3 test_spi.py
```

Debe terminar con `¡ÉXITO! COMUNICACIÓN SPI REAL CONFIRMADA`. Si dice «No se recibió cabecera válida», vuelve a ejecutarlo.

> El modo de prueba **se queda activo** en el ESP32-C6 hasta que lo reinicies. Después de `test_spi.py`, **pulsa el botón RESET** del ESP32-C6 para volver a leer el potenciómetro. Si no, el dashboard mostrará «modo test» con dos líneas casi planas.

---

## 8. Paso 6: encender el sistema y abrir el dashboard

### 8.1 Arranca el backend

En la sesión SSH de la BeaglePlay:

```bash
cd ~/Osciloscopio-Linux-Embebido/beagleplay/backend
SPI_SPEED_HZ=2000000 sudo -E python3 -m uvicorn main:app --host 0.0.0.0 --port 8000
```

Qué significa: `SPI_SPEED_HZ=2000000` usa el SPI a 2 MHz (la velocidad que funcionó; a 100 kHz falla con más frecuencia) y `sudo -E` da permiso de acceso al SPI sin perder esa variable.

**Resultado esperado:**

```
INFO:     Application startup complete.
INFO:     Uvicorn running on http://0.0.0.0:8000 (Press CTRL+C to quit)
```

**Deja esta ventana abierta.** Si la cierras, el sistema se detiene. Si ves `[SPI Driver Error] No se pudo abrir la interfaz SPI`, vuelve a la sección 5.4.

### 8.2 Abre el dashboard

En el **navegador de tu PC** escribe:

```
http://192.168.7.2:8000
```

(Si conectaste la BeaglePlay por Ethernet, usa su IP en lugar de `192.168.7.2`; en la BeaglePlay se ve con `hostname -I`.)

**Resultado esperado:** una página oscura con el título «Osciloscopio · BeaglePlay + ESP32-C6», una gráfica vacía y una tabla de estadísticas.

---

## 9. Cómo usar el dashboard

![Dashboard funcionando](../Documentaci%C3%B3n_General/img/dashboard-con-condensador.png)

### 9.1 Primera medición

1. En **Fuente**, elige **API real (/api/scope/frame)**. («Simulada» dibuja señales falsas y no usa el hardware.)
2. Pulsa **▶ Start**. El botón pasa a **■ Stop** y la insignia verde dice «capturando».
3. En unos segundos aparecen dos líneas: **CH A** (azul) y **CH B** (amarillo).
4. **Gira la perilla del potenciómetro** despacio: la línea azul sube y baja.

### 9.2 Qué significa cada cosa

| Elemento | Significado |
|---|---|
| **Eje vertical** | Voltaje en voltios (de 0 a 3,3 V) |
| **Eje horizontal** | Tiempo en milisegundos dentro de la ventana de captura |
| **CH A / CH B** | Las dos señales medidas (GPIO0 y GPIO1) |
| **Mín / Máx / Pico-pico / Prom / RMS** | Voltaje más bajo, más alto, diferencia entre ambos, promedio y valor eficaz de la ventana |
| Barra inferior | Número de captura (`seq`), muestras por segundo, número de muestras, duración de la ventana y avisos |

Con el potenciómetro, la señal es **continua** (no cambia con el tiempo): verás una **línea horizontal**. En **Prom** lees su voltaje. Con 1 V real, el dashboard debe marcar cerca de 1 V (ver precisión en la sección 11).

### 9.3 Controles

| Control | Qué hace |
|---|---|
| **▶ Start / ■ Stop** | Inicia o detiene las capturas |
| **Frecuencia de muestreo** | Cuántas muestras por segundo: 100, 1.000, 10.000 o 40.000. Cambia la «velocidad de la cámara» del instrumento. **Con 40.000 hay un error conocido** (sección 11) |
| **Refresco** | Cada cuánto se pide una captura nueva: 0,25 Hz (4 s), 0,5 Hz (2 s) o 1 Hz (1 s) |

La ventana siempre tiene 256 muestras, así que su **duración** depende de la frecuencia de muestreo:

| Frecuencia | Duración de la ventana (256 muestras) |
|---|---|
| 100 Sa/s | 2,56 s |
| 1.000 Sa/s | 256 ms |
| 10.000 Sa/s | 25,6 ms |
| 40.000 Sa/s | 6,4 ms |

A 100 Sa/s cada captura dura 2,56 s, por lo que el refresco real es de unos 0,38 Hz aunque elijas 1 Hz.

### 9.4 Avisos de la barra inferior

| Aviso | Qué significa |
|---|---|
| `modo test` | El ESP32-C6 está en modo de prueba. Pulsa **RESET** en el ESP32-C6 |
| `reintentos SPI: N` | Hubo N lecturas SPI perdidas y repetidas en esa captura. Es normal que haya algunas |
| `OVERRUN` | El ADC perdió muestras (suele pasar a 40.000 Sa/s) |
| `comando rechazado` | Se intentó cambiar la configuración en pleno muestreo; se repite solo |
| `ADC sin calibrar` | Esa placa no trae datos de calibración; los voltajes son aproximados |
| `error: HTTP 500` (insignia roja) | El backend no pudo leer el ESP32-C6. Mira el mensaje en la ventana del backend (sección 10) |

---

## 10. Problemas comunes y soluciones

| Síntoma | Causa probable | Qué hacer |
|---|---|---|
| `Failed to connect` al flashear | La placa no entra en modo de descarga o el cable es solo de carga | Cambia el cable. Mantén **BOOT**, pulsa **RESET**, suelta **BOOT** y repite |
| No aparece el puerto COM / `/dev/ttyACM0` | Cable sin datos, o WSL sin `usbipd` | Otro cable. En WSL, `usbipd attach` (ver [05](../Documentaci%C3%B3n_General/05-firmware-esp32c6.md)) |
| `ssh` no conecta a 192.168.7.2 | La BeaglePlay no terminó de arrancar, o falta el driver de red USB | Espera 2 minutos. En Windows revisa el driver USB. O usa Ethernet |
| No existe `/dev/spidev0.0` | La imagen no trae el SPI del mikroBUS habilitado | Consulta al profesor; ver [12](../Documentaci%C3%B3n_General/12-guia-equipo-sbc.md) |
| `ModuleNotFoundError: fastapi` (o `spidev`, `uvicorn`) | Faltan paquetes de Python para `sudo python3` | Sección 5.5 |
| `prueba_directa_spi.py` solo da `FF` en las 5 pruebas | Cable de MISO, MOSI, SCK o CS mal puesto; sin GND común; ESP32-C6 sin alimentar | Apaga todo, revisa la tabla de la sección 6.1 y repite |
| Funciona y falla de forma intermitente | Problema conocido: se pierde cerca de la mitad de las lecturas SPI; el software reintenta | Normal. Usa cables cortos y bien insertados; usa **2 MHz** |
| Dashboard con `error: HTTP 500` | Se agotaron los reintentos SPI o el ESP32-C6 no responde | Mira el mensaje `[ERROR REST] ...` en la ventana del backend. Reinicia el ESP32-C6 (RESET) y recarga |
| Dashboard dice `modo test` y dos líneas casi planas | El ESP32-C6 sigue en modo de prueba | Pulsa **RESET** en el ESP32-C6 |
| La gráfica muestra ruido o caídas periódicas en CH A | Entrada analógica sin filtro | Revisa el condensador y la resistencia de 1 kΩ. Mantén los cables de la señal lejos de los cables SPI |
| Valores siempre ≈ 0 V o ≈ 3,1 V | Señal fuera de rango, sin GND común, o pin sin conectar | Revisa el potenciómetro y que GND del potenciómetro vaya al GND del ESP32-C6 |
| Los voltajes no coinciden con el multímetro | Calibración de fábrica limitada del ADC | Ver sección 11 |
| La página no carga en el navegador | El backend no está corriendo o la IP es otra | Comprueba la ventana del backend y la IP |
| El dashboard muestra la fuente «Simulada» | Es la opción por defecto | Cambia **Fuente** a «API real» |

---

## 11. Limitaciones conocidas

Para que sepas qué esperar (todas están en [13-pendientes-y-riesgos.md](../Documentaci%C3%B3n_General/13-pendientes-y-riesgos.md)):

1. **40.000 Sa/s falla:** con ese preset aparece un error constante; no está resuelto. Funcionan 100, 1.000 y 10.000 (este último con errores esporádicos).
2. **Se pierde cerca de la mitad de las lecturas SPI.** El software las repite automáticamente; sólo notarás algo más de espera y el aviso `reintentos SPI`.
3. **Precisión del voltaje.** El ADC del ESP32-C6 sin calibrar se equivocó hasta 0,7 V frente a un multímetro. El firmware v2 incluye calibración de fábrica, pero **aún no se ha medido en hardware**: no confíes en el valor absoluto sin compararlo con un multímetro.
4. **Sin disparo (trigger):** cada captura empieza cuando se pide; una señal que cambia en el tiempo aparecerá en fases distintas en cada captura.
5. **Servidor web:** el dashboard lo sirve el propio backend de Python, no apache ni nginx (el enunciado del proyecto pide uno de los dos; está pendiente).
6. **Rango de entrada:** de 0 a unos 3,1 V, solo positivo.

---

## 12. Apagar y guardar

1. En la ventana del backend pulsa `Ctrl + C`.
2. Apaga la BeaglePlay: `sudo poweroff`. Espera ~10 segundos y desconecta su USB-C.
3. Desconecta el USB del ESP32-C6.
4. Después puedes desmontar los cables.

Para volver a usarlo: monta, repite desde el **paso 5** (sección 7) y arranca el backend (sección 8). **No hace falta volver a flashear** el ESP32-C6.

---

## 13. Resumen de comandos

| Dónde | Para qué | Comando |
|---|---|---|
| PC | Copiar el repositorio | `git clone https://github.com/dahenaor-source/Osciloscopio-Linux-Embebido.git` |
| PC | Instalar esptool | `py -m pip install esptool` (Windows) · `pip install esptool` (Linux) |
| PC | Flashear el ESP32-C6 | `py -m esptool --chip esp32c6 -p COM5 -b 460800 write_flash 0x0 release\osciloscopio_c6_v2.bin` |
| PC | Ver el monitor serie | `py -m serial.tools.miniterm COM5 115200` |
| PC | Copiar el repositorio a la BeaglePlay | `scp -r Osciloscopio-Linux-Embebido USUARIO@192.168.7.2:~/` |
| PC | Entrar a la BeaglePlay | `ssh USUARIO@192.168.7.2` |
| BeaglePlay | Ver el puerto SPI | `ls /dev/spidev*` |
| BeaglePlay | Probar el SPI | `sudo python3 prueba_directa_spi.py` (en `beagleplay/backend`) |
| BeaglePlay | Arrancar el sistema | `SPI_SPEED_HZ=2000000 sudo -E python3 -m uvicorn main:app --host 0.0.0.0 --port 8000` |
| Navegador | Abrir el dashboard | `http://192.168.7.2:8000` |
| BeaglePlay | Apagar | `sudo poweroff` |

## Más documentación

- Descripción técnica completa: [Documentación General](../Documentaci%C3%B3n_General/README.md).
- Protocolo SPI: [04-protocolo-spi.md](../Documentaci%C3%B3n_General/04-protocolo-spi.md).
- Qué está probado y qué no: [09-pruebas-y-resultados.md](../Documentaci%C3%B3n_General/09-pruebas-y-resultados.md).
