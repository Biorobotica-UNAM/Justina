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
SUBSYSTEM=="tty", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6001", ATTRS{serial}=="A900fDgq", SYMLINK+="head", MODE="0666"
SUBSYSTEM=="tty", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6001", ATTRS{serial}=="A5052R8H", SYMLINK+="leftArm", MODE="0666"
SUBSYSTEM=="tty", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6001", ATTRS{serial}=="A7005LgE", SYMLINK+="rightArm", MODE="0666"
SUBSYSTEM=="tty", ATTRS{idVendor}=="15d1", ATTRS{idProduct}=="0000", SYMLINK+="lidar", MODE="0666"
SUBSYSTEM=="tty", ATTRS{idVendor}=="1ffb", ATTRS{idProduct}=="0085", ATTRS{serial}=="00191339", ENV{ID_USB_INTERFACE_NUM}=="00", SYMLINK+="torso", MODE="0666"
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

chmod +x scripts/justina_on.sh
echo "alias hardware_on='~/Documents/devJ/Justina/scripts/hardware_on.sh'" >> ~/.bashrc

# Actualizar lista de paquetes
echo "Actualizando lista de paquetes..."
sudo apt update

sudo apt-get update && sudo apt-get install -y sshpass

# Instalar dependencias ROS y del sistema
echo "Instalando dependencias ROS 2 y de sistema..."
sudo apt install -y \
  ros-jazzy-control-msgs \
  ros-jazzy-urg-node \
  ros-jazzy-nav2-map-server \
  ros-jazzy-nav2-amcl \
  ros-jazzy-xacro \
  ros-jazzy-nav2-lifecycle-manager \
  ros-jazzy-urg-node \
  ros-jazzy-depth-image-proc \
  ros-jazzy-camera-info-manager \
  ros-jazzy-tf-transformations \
  ros-jazzy-dynamixel-sdk \
  espeak-ng \
  nlohmann-json3-dev \
  portaudio19-dev \
  flite


# --- Configuración de CycloneDDS y Alias ---

# 1. Mover el archivo XML a la raíz (pide contraseña de sudo)
echo "Configurando cyclonedds.xml en la raíz..."
sudo mv cyclonedds.xml /cyclonedds.xml
sudo chmod 644 /cyclonedds.xml

# 2. Agregar alias al .bashrc solo si no existen ya
if ! grep -q "alias justina_on=" ~/.bashrc; then
    echo "Agregando alias justina_on y justina_off a ~/.bashrc..."
    echo "" >> ~/.bashrc
    echo "# Justina Robot Aliases" >> ~/.bashrc
    echo "alias justina_on='export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp; export CYCLONEDDS_URI=/cyclonedds.xml; export ROS_DOMAIN_ID=0; export ROS_LOCALHOST_ONLY=0'" >> ~/.bashrc
    echo "alias justina_off='unset CYCLONEDDS_URI; unset RMW_IMPLEMENTATION; unset ROS_DOMAIN_ID; unset ROS_LOCALHOST_ONLY'" >> ~/.bashrc
    echo "Alias agregados correctamente."
else
    echo "Los alias ya existen en ~/.bashrc, omitiendo paso."
fi

echo "Configuración terminada. Por favor, corre 'source ~/.bashrc' o abre una nueva terminal."

source ~/.bashrc

cd /tmp
git clone https://github.com/OpenKinect/libfreenect.git
cd libfreenect
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make
sudo make install
sudo ldconfig

# Instalar dependencias de Python
pip3 install openai clipspy vosk pyaudio noisereduce --break-system-packages

echo "Instalación completada correctamente."
