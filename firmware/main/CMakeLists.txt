idf_component_register(SRCS "main.c" "scope_link.c" "adc_capture.c"
                       INCLUDE_DIRS "."
                       REQUIRES driver esp_adc esp_timer scope_proto)
