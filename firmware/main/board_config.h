#pragma once

/* Pines del ESP32-C6-DevKitC-1. Ver docs/hardware-pinout.md */

/* SPI2 (GPSPI2) con los pines nativos del IO_MUX, necesarios para alta velocidad en modo esclavo */
#define PIN_SPI_SCLK 6   /* FSPICLK  <- mikroBUS SCK  */
#define PIN_SPI_MOSI 7   /* FSPID    <- mikroBUS MOSI */
#define PIN_SPI_MISO 2   /* FSPIQ    -> mikroBUS MISO */
#define PIN_SPI_CS 10    /* CS por matriz GPIO (el CS nativo, GPIO16, es el TX de la UART de consola) */

/* Salida de handshake: nivel alto = el esclavo tiene una transacción lista. -> mikroBUS INT */
#define PIN_HANDSHAKE 3

/* Entradas analógicas (ADC1) */
#define ADC_CH_A ADC_CHANNEL_0 /* GPIO0 */
#define ADC_CH_B ADC_CHANNEL_1 /* GPIO1 */
