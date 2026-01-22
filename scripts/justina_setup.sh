#!/bin/bash
# ===========================================================================
# Script para instalar dependencias ROS 2 Jazzy y crear alias de dispositivos
# ===========================================================================

########################################
# Crea alias para el brazo y la cabeza #
########################################

echo "Creando alias de udev para las tarjetas de Justina..."

# Archivo de reglas
RULES_FILE="/etc/udev/rules.d/99-usb-alias.rules"

# Crear el archivo (no necesita chmod 666)
sudo touch $RULES_FILE

# Escribir reglas correctas
sudo tee $RULES_FILE > /dev/null << EOF
SUBSYSTEM=="tty", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6001", ATTRS{serial}=="A900fDgq", SYMLINK+="justinaRightArm", MODE="0666"
SUBSYSTEM=="tty", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6001", ATTRS{serial}=="A7005LgE", SYMLINK+="justinaHead", MODE="0666"
EOF

# Recargar reglas
sudo udevadm control --reload-rules
sudo udevadm trigger

echo "Alias creados:"
echo "  /dev/justinaRightArm"
echo "  /dev/justinaHead"

############################################
# Descarga las dependencias de ROS 2 Jazzy #
############################################

echo "Iniciando instalación de paquetes ROS 2 Jazzy..."

# Verificar si ROS 2 Jazzy está instalado
if ! printenv | grep -q "ROS_DISTRO=jazzy"; then
  echo "No se detectó ROS 2 Jazzy. Asegúrate de tenerlo instalado y haber hecho 'source /opt/ros/jazzy/setup.bash'"
  exit 1
fi

# Actualizar lista de paquetes
echo "Actualizando lista de paquetes..."
sudo apt update

# Instalar dependencias ROS y del sistema
echo "Instalando dependencias ROS 2 y de sistema..."
sudo apt install -y \
  ros-jazzy-control-msgs \
  ros-jazzy-urg-node \
  ros-jazzy-nav2-map-server \
  ros-jazzy-nav2-amcl \
  ros-jazzy-xacro \
  ros-jazzy-nav2-lifecycle-manager \
  espeak-ng

echo "Instalación completada correctamente."
