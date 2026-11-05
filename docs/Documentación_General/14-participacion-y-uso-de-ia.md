# 14. Participación y uso de inteligencia artificial

## 1. Participación del equipo

El proyecto integra firmware para ESP32-C6, comunicación SPI,
backend FastAPI en BeaglePlay, dashboard web y documentación
técnica. La distribución de funciones registrada es:

| Integrante | Área de participación |
|---|---|
| David Henao Rojas | Firmware ESP32-C6, protocolo SPI, calibración ADC, primera versión del dashboard, pruebas unitarias y documentación técnica |
| Juan Camilo Giraldo | Software de la BeaglePlay, integración del backend FastAPI, comunicación SPI y pruebas de integración |
| Laura Daniela Barragán Silva | Preparación del entorno Ubuntu y ESP-IDF; compilación y diagnóstico de programación del ESP32-C6; revisión del enlace y de las incidencias de comunicación; gestión de ramas Git; actualización y depuración del repositorio; revisión del manual, README, matriz de verificación y documentación técnica |

La atribución anterior describe la distribución documentada
y no pretende asignar autoría exclusiva sobre archivos
desarrollados de forma colaborativa.

El historial de Git conserva la trazabilidad de los cambios.


## 2. Aportes de Laura Daniela Barragán Silva

La participación documentada comprende actividades de
integración, diagnóstico, gestión del repositorio y
documentación del proyecto.

### Entorno de desarrollo y ESP32-C6

Se trabajó desde Ubuntu con ESP-IDF para preparar la
compilación del firmware de las ramas de desarrollo.
Se revisaron errores de compilación relacionados con
la ubicación del proyecto y la activación del entorno.

También se diagnosticaron dificultades de programación
por USB, incluyendo cambios entre `/dev/ttyACM0` y
`/dev/ttyACM1`, desaparición temporal del dispositivo
serie y posibles ocupaciones del puerto.

Los registros de compilación satisfactoria se distinguen
de los intentos de flasheo fallidos. No se considera
programada una placa por el solo hecho de generar
correctamente el archivo binario.

### Integración y diagnóstico

Se revisaron registros del firmware, inicialización de
SPI, calibración eFuse, errores de adquisición y
comportamiento del dashboard.

Se trabajó sobre la interpretación de las versiones del
protocolo, el modo de simulación y la adquisición real,
y la coherencia entre los resultados del backend y las
evidencias experimentales.

### Gestión de versiones

Se realizaron operaciones de actualización, comparación
y cambio entre las ramas `Osc-V2`, `Osc-V3` y `main`.
Se respaldó el historial local cuando existieron ramas
divergentes y se integraron modificaciones documentales
respetando los commits de los demás integrantes.

Se realizaron commits y publicaciones de documentación
en GitHub, así como la depuración de documentos
redundantes y corrección de sus referencias internas.

### Documentación técnica

Se revisaron y organizaron:

- README principal y documentos de los componentes.
- Manual de instalación y operación.
- Guías de ESP32-C6 y BeaglePlay.
- Documentación del protocolo SPI.
- Bitácora de pruebas y matriz de verificación.
- Resultados de calibración y sus limitaciones.
- Declaración de participación y uso de IA.
- Estructura y enlaces del repositorio.

Se distinguieron resultados históricos, pruebas
funcionales actuales y verificaciones parciales.

Las mediciones identificadas como P-18 a P-22 se citan
como evidencias del proyecto, sin atribuir su ejecución
individual a partir de esta revisión documental.

## 3. Uso de inteligencia artificial en estos aportes

Se utilizó **ChatGPT** como herramienta de asistencia
para analizar registros de terminal, errores de Git,
flasheo y comunicación, interpretar resultados y
preparar procedimientos de revisión documental.

También se utilizó para elaborar y revisar comandos
de actualización, consolidar información técnica y
detectar contradicciones entre documentos.

Las sugerencias generadas con asistencia de IA
se contrastaron con los resultados proporcionados
por las herramientas de desarrollo y las evidencias
publicadas en el repositorio.

La IA no reemplazó las verificaciones experimentales,
la ejecución de comandos ni la revisión de sus
resultados.

## 4. Uso de inteligencia artificial en el proyecto

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

## 5. Verificación de resultados

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

## 6. Limitaciones de la documentación

La atribución individual de todas las intervenciones de IA,
sus modelos específicos y sus historiales completos no
puede reconstruirse únicamente a partir de los archivos
actualmente publicados.

Los historiales de conversaciones no están incorporados
como anexos en esta versión del repositorio.

Las mediciones parciales se identifican como tales
en la matriz y no se presentan como certificaciones
metrológicas completas.
