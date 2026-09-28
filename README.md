# Pumas@HOME
This is the official repository of Team Pumas' Robot Justina for the RoboCup@HOME.

# Install
After copying the repository, run inside the repository folder _~/Justina/_:
```bash
git submodule update --init --recursive
```
This clone all submodules git, for example _freenect_stack_

# Installation of ROS2 Jazzy dependencies
To install all the dependencies needed for the Justina_temp project, follow these steps:
1. Go to the main folder of the repository (or clone it if you don't have it yet):
```bash
git clone https://github.com/Biorobotica-UNAM/Justina.git
cd Justina
```
2. Give the installation script execution permissions:
```bash
chmod +x scripts/install_ros2_packages.sh
```
3. Run the script to install the dependencies:
```bash
bash scripts/install_ros2_packages.sh
```
# Troubleshooting
In case the known locations don't appear in the map, when launched. Add the following lines to _.bashrc_:
```bash
export JUSTINA_WS="$HOME/biorobotica-unam/Justina"

source /opt/ros/jazzy/setup.bash
source "$JUSTINA_WS/install/setup.bash"

export PYTHONPATH="$JUSTINA_WS/src/Navigation/navigation_controller/locations_manager:$PYTHONPATH"
```
and source it.