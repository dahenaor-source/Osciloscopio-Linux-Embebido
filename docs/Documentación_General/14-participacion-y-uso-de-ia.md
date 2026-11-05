# 14. Participación y uso de inteligencia artificial

## 1. Participación del equipo

El proyecto integra firmware para ESP32-C6, comunicación SPI,
backend FastAPI en BeaglePlay, dashboard web y documentación
técnica. La distribución de funciones registrada es:

| Integrante | Área de participación |
|---|---|
| David Henao Rojas | Firmware ESP32-C6, protocolo SPI, calibración ADC, primera versión del dashboard, pruebas unitarias y documentación técnica |
| Juan Camilo Giraldo | Software de la BeaglePlay, integración del backend FastAPI, comunicación SPI y pruebas de integración |
| Daniela Barragán | Integrante del equipo de software de la BeaglePlay; el repositorio no individualiza suficientemente sus contribuciones técnicas |

La atribución anterior describe la distribución documentada
y no pretende asignar autoría exclusiva sobre archivos
desarrollados de forma colaborativa.

El historial de Git conserva la trazabilidad de los cambios.

## 2. Uso de inteligencia artificial

En el desarrollo se registró el uso de asistentes de IA,
entre ellos GitHub Copilot, Gemini y ChatGPT.

Las actividades documentadas incluyen apoyo para:

- Implementación y revisión de firmware y protocolo SPI.
- Desarrollo de herramientas de diagnóstico.
- Elaboración y revisión de la interfaz web.
- Interpretación de registros de pruebas.
- Elaboración de diagramas y documentación técnica.

El uso de estas herramientas no constituye por sí mismo
evidencia de funcionamiento del sistema.

## 3. Verificación de resultados

Los resultados generados o revisados con asistencia de IA
se contrastaron, según el caso, mediante:

- Compilación del firmware con ESP-IDF.
- Pruebas unitarias del protocolo.
- Ejecución en ESP32-C6 y BeaglePlay.
- Pruebas de integración SPI y API.
- Verificación del servidor nginx y el arranque automático.
- Mediciones con generador de señales y multímetro.

La evidencia experimental y sus limitaciones se encuentran en:

- [09. Pruebas y resultados](09-pruebas-y-resultados.md).
- [10. Matriz de verificación](10-matriz-de-verificacion.md).

## 4. Limitaciones de la documentación

La atribución individual de todas las intervenciones de IA,
sus modelos específicos y sus historiales completos no
puede reconstruirse únicamente a partir de los archivos
actualmente publicados.

Los historiales de conversaciones no están incorporados
como anexos en esta versión del repositorio.

Las mediciones parciales se identifican como tales
en la matriz y no se presentan como certificaciones
metrológicas completas.
