#pragma once

#include "esp_err.h"

/* Inicializa el SPI esclavo y las tareas de enlace y captura. */
esp_err_t scope_link_start(void);
