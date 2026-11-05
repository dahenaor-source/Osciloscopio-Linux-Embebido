#!/usr/bin/env bash
# Instala el backend del osciloscopio como servicio de systemd: al alimentar la BeaglePlay arranca solo
# (nginx ya arranca solo) y el dashboard queda disponible en http://<IP de la BeaglePlay>/
#
# Uso, en la BeaglePlay, desde cualquier carpeta:
#   sudo bash ~/Osciloscopio-Linux-Embebido/beagleplay/instalar_servicio.sh
# Se puede ejecutar de nuevo sin problema (por ejemplo, tras mover el repositorio).
set -euo pipefail

if [ "$(id -u)" -ne 0 ]; then
    echo "Ejecuta con sudo:  sudo bash $0" >&2
    exit 1
fi

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
USUARIO="${SUDO_USER:-$(logname 2>/dev/null || echo root)}"
HOME_USUARIO="$(getent passwd "$USUARIO" | cut -d: -f6)"
PYTHON="$(command -v python3)"
PLANTILLA="$REPO/beagleplay/systemd/osciloscopio-backend.service"
SERVICIO=osciloscopio-backend
DESTINO="/etc/systemd/system/$SERVICIO.service"

echo "Repositorio: $REPO"
echo "Usuario:     $USUARIO ($HOME_USUARIO)"
echo "Python:      $PYTHON"

[ -f "$PLANTILLA" ] || { echo "No existe $PLANTILLA" >&2; exit 1; }
[ -f "$REPO/beagleplay/backend/main.py" ] || { echo "No existe beagleplay/backend/main.py en $REPO" >&2; exit 1; }

# Los paquetes de Python deben verse igual que los verá el servicio (como root, con ~/.local del usuario).
if ! PYTHONUSERBASE="$HOME_USUARIO/.local" "$PYTHON" -c "import fastapi, uvicorn, spidev" 2>/dev/null; then
    echo "Faltan paquetes de Python (fastapi, uvicorn, spidev). Instálalos y vuelve a ejecutar:" >&2
    echo "  sudo apt update && sudo apt install -y python3-pip python3-dev" >&2
    echo "  pip3 install fastapi uvicorn spidev --break-system-packages    (como $USUARIO, sin sudo)" >&2
    exit 1
fi

if [ ! -e /dev/spidev0.0 ]; then
    echo "AVISO: no existe /dev/spidev0.0. El servicio arrancará, pero no leerá el ESP32-C6 hasta que aparezca." >&2
fi

# Un uvicorn lanzado a mano ocuparía el puerto 8000 y el servicio no podría arrancar.
if ! systemctl is-active --quiet "$SERVICIO" && ss -ltnH 'sport = :8000' | grep -q .; then
    echo "El puerto 8000 está ocupado (¿tienes uvicorn corriendo a mano?). Detenlo con Ctrl+C y repite." >&2
    exit 1
fi

sed -e "s|@REPO@|$REPO|g" -e "s|@HOME@|$HOME_USUARIO|g" -e "s|@PYTHON@|$PYTHON|g" "$PLANTILLA" > "$DESTINO"
chmod 644 "$DESTINO"
systemd-analyze verify "$DESTINO" || echo "AVISO: systemd-analyze reportó advertencias (revisa arriba)." >&2

systemctl daemon-reload
systemctl enable "$SERVICIO"
systemctl restart "$SERVICIO"
sleep 3

if systemctl is-active --quiet "$SERVICIO"; then
    echo "OK: el servicio $SERVICIO está activo y se iniciará solo al encender la placa."
else
    echo "ERROR: el servicio no quedó activo. Últimas líneas del registro:" >&2
    journalctl -u "$SERVICIO" -n 20 --no-pager >&2
    exit 1
fi

systemctl is-active --quiet nginx || echo "AVISO: nginx no está activo; el dashboard por el puerto 80 no responderá (sudo systemctl enable --now nginx)." >&2

echo
echo "Comprobar:  curl -i http://127.0.0.1/api/scope/frame     (200 = todo bien)"
echo "Registro:   sudo journalctl -u $SERVICIO -f"
echo "Dashboard:  http://<IP de la BeaglePlay>/   (por USB: http://192.168.7.2/)"
