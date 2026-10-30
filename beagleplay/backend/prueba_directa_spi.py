import spidev
import time


print("=== DIAGNOSTICO DIRECTO SPI ===")


spi = spidev.SpiDev()


try:
    spi.open(0, 0)


    # Prueba inicial a 100 kHz
    spi.max_speed_hz = 100000
    spi.mode = 0
    spi.bits_per_word = 8


    print("Bus: /dev/spidev0.0")
    print("Frecuencia:", spi.max_speed_hz)
    print("Modo:", spi.mode)


    # Comando NOP valido del protocolo
    comando = [0x00, 0x00, 0x00, 0x5A]


    paquete = comando + [0x00] * 12


    for intento in range(5):


        tx = paquete.copy()


        print(f"\nPRUEBA {intento + 1}")
        print("TX:", [f"{x:02X}" for x in tx])


        rx = spi.xfer2(paquete.copy())


        print("RX:", [f"{x:02X}" for x in rx])


        if len(rx) == 16 and rx[0] == 0xE5:


            checksum = 0


            for byte in rx[:15]:
                checksum ^= byte


            if checksum == rx[15]:
                print("SPI OK: cabecera valida")
                break


            print("ERROR: checksum incorrecto")


        else:
            print("Sin respuesta valida del ESP32-C6")


        time.sleep(0.5)


finally:
    spi.close()