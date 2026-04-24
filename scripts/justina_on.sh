#!/bin/bash

PASS="robotino"
USER_HOST="robotino@robotino"

echo "--- Conectando a Robotino y encendiendo Hardware ---"

sshpass -p "$PASS" ssh $USER_HOST << 'EOF'
  echo "Iniciando contenedor de hardware..."
  docker run --rm -i \
    --network host \
    --ipc host \
    --pid host \
    --privileged \
    -e RMW_IMPLEMENTATION=rmw_cyclonedds_cpp \
    -e ROS_DOMAIN_ID=0 \
    -v /dev:/dev \
    -v /run/udev:/run/udev:ro \
    justina_hardware:jazzy \
    bash -c "source /opt/ros/jazzy/setup.bash && ros2 launch justina_hardware hardware.launch.xml"
EOF