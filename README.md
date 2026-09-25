# Robot de Servicio Justina — Manual de Inicio Rápido y Guía de Operación
**Team Pumas @Home — Laboratorio de Bio-robótica, UNAM**  
*Plataforma ROS 2 Jazzy | Ubuntu 24.04*

---

## Tabla de Contenidos
1. [Instalación y Compilación del Repositorio](#1-instalación-y-compilación-del-repositorio)
2. [Configuración de Red y DDS (Comunicación con Robotino)](#2-configuración-de-red-y-dds-comunicación-con-robotino)
3. [Encendido de Hardware y Docker en Robotino](#3-encendido-de-hardware-y-docker-en-robotino)
4. [Lanzamiento del Sistema Principal (Surge et Ambula)](#4-lanzamiento-del-sistema-principal-surge-et-ambula)
5. [Panel Gráfico de Control y Pruebas Completas (GUI Controller)](#5-panel-gráfico-de-control-y-pruebas-completas-gui-controller)
6. [Pruebas Básicas y Monitoreo](#6-pruebas-básicas-y-monitoreo)
7. [Ejecución de Servicios (Services)](#7-ejecución-de-servicios-services)
8. [Ejecución de Acciones (Actions con Executor Worker)](#8-ejecución-de-acciones-actions-con-executor-worker)
9. [Catálogo de Tópicos Principales](#9-catálogo-de-tópicos-principales)
10. [Modo Simulación](#10-modo-simulación)

---

## 1. Instalación y Compilación del Repositorio

Para nuevos integrantes o configuraciones en una computadora de control:

### 1.1 Clonar el repositorio con submódulos
El proyecto utiliza submódulos de Git para cada módulo funcional. Debe clonarse de forma recursiva:
```bash
git clone --recurse-submodules https://github.com/Biorobotica-UNAM/Justina.git ~/Justina
cd ~/Justina
git submodule update --init --recursive
```

### 1.2 Ejecutar script de configuración y dependencias
El script `scripts/justina_setup.sh` realiza las siguientes configuraciones clave:
* Crea reglas udev (`/etc/udev/rules.d/99-usb-alias.rules`) para asignar puertos fijos a los dispositivos USB del robot (`/dev/head`, `/dev/leftArm`, `/dev/rightArm`, `/dev/lidar`, `/dev/torso`).
* Instala dependencias apt de ROS 2 Jazzy (`moveit`, `nav2`, `urg-node`, `dynamixel-sdk`, `cyclonedds`, etc.).
* Compila e instala `libfreenect` para la cámara Kinect.
* Instala dependencias de Python (`openai`, `clipspy`, `vosk`, `pyaudio`, `noisereduce`).
* Agrega los alias de conveniencia a `~/.bashrc` (`justina_on`, `justina_off`, `hardware_on`).

Ejecuta el script:
```bash
chmod +x scripts/justina_setup.sh
./scripts/justina_setup.sh
```

### 1.3 Compilar el espacio de trabajo
Carga el entorno de ROS 2 Jazzy y compila con `colcon`:
```bash
source /opt/ros/jazzy/setup.bash
colcon build --symlink-install
source install/setup.bash
```

---

## 2. Configuración de Red y DDS (Comunicación con Robotino)

La base móvil de Justina (Robotino) se comunica mediante una red Ethernet o WiFi dedicada con dirección IP estática.

### 2.1 Direccionamiento IP
* **IP de Robotino:** `172.27.1.1`
* **Hostname:** `robotino`
* **Máscara:** `255.255.255.0` (`/24`)

Verifica que tu archivo `/etc/hosts` contenga la resolución de nombre:
```bash
cat /etc/hosts | grep robotino
# Debe devolver:
# 172.27.1.1 robotino
```
Si no existe, agrégala con:
```bash
sudo sh -c 'echo "172.27.1.1 robotino" >> /etc/hosts'
```

### 2.2 Configurar la interfaz de red en tu PC
Debes asignar a tu tarjeta Ethernet una IP en el rango `172.27.1.X` (por ejemplo, `172.27.1.100`):

#### Opción A: Por terminal (NetworkManager / nmcli)
Identifica tu interfaz de red:
```bash
ip link
```
Configura la conexión estática (reemplaza `enp3s0` por el nombre de tu tarjeta):
```bash
sudo nmcli con add type ethernet con-name "Robotino-Eth" ifname enp3s0 ip4 172.27.1.100/24
sudo nmcli con up "Robotino-Eth"
```

#### Opción B: Por interfaz gráfica (Ubuntu Settings)
1. Abre **Settings** -> **Network** -> **Wired**.
2. Ve a la pestaña **IPv4**.
3. Selecciona **Manual**:
   * **Address:** `172.27.1.100`
   * **Netmask:** `255.255.255.0`
   * **Gateway:** `172.27.1.1` (opcional)
4. Aplica y reconecta el cable.

### 2.3 Comprobar la conexión
```bash
ping -c 4 robotino
```
Si recibes respuesta, la conexión física y de red está lista.

### 2.4 Configurar CycloneDDS y ROS 2
En ROS 2 Jazzy, Justina utiliza `rmw_cyclonedds_cpp` en el `ROS_DOMAIN_ID=0`. Para cargar las variables de entorno en cada terminal:
```bash
justina_on
```
> **Nota:** Si aún no tienes el alias en tu `~/.bashrc`, ejecuta manualmente:
> ```bash
> export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
> export CYCLONEDDS_URI=/cyclonedds.xml
> export ROS_DOMAIN_ID=0
> export ROS_LOCALHOST_ONLY=0
> ```

---

## 3. Encendido de Hardware y Docker en Robotino

El control de motores y drivers de bajo nivel de la base reside en un contenedor Docker (`justina_hardware:jazzy`) dentro del equipo de Robotino.

### Método 1: Script automatizado (`justina_on.sh`)
Desde la raíz del repositorio en tu laptop/PC principal:
```bash
bash scripts/justina_on.sh
```
*(O usa el alias `hardware_on` creado por `justina_setup.sh`).*

Este script se conecta vía SSH a `robotino@robotino` (contraseña: `robotino`) e inicia el contenedor con los permisos y dispositivos necesarios.

### Método 2: Acceso SSH manual
Si necesitas ingresar directamente a la base:
```bash
ssh robotino@robotino
# Contraseña: robotino
```

Dentro de Robotino, puedes correr el contenedor manualmente:
```bash
docker run --rm -it \
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
```

Una vez que el hardware está encendido, en tu máquina host debes ver los tópicos `/hardware/base/cmd_vel`, `/hardware/base/odom`, `/hardware/base/scan`, etc.

---

## 4. Lanzamiento del Sistema Principal (Surge et Ambula)

El paquete `surge_et_ambula` es el orquestador principal de Justina. Se encarga de levantar el Supervisor, Executor, Planning (CLIPS), Navegación, Manipulación (MoveIt), Visión, HRI y RViz2.

### 4.1 Lanzamiento en Robot Real
En una terminal en tu PC (con el entorno configurado con `justina_on` y el workspace fuenteado):
```bash
source install/setup.bash
ros2 launch surge_et_ambula justina.launch.xml
```

#### Argumentos y banderas disponibles:
| Argumento | Default | Descripción |
|---|---|---|
| `use_nav` | `true` | Habilita o deshabilita la pila completa de navegación. |
| `mapless` | `false` | `false`: Navegación AMCL con mapa precargado. `true`: Modo SLAM/mapless. |
| `plan_name` | `default_plan` | Nombre del plan a cargar en Planning y CLIPS. |
| `ask_data_confirmation_enabled` | `true` | Solicitar confirmación de datos en el Supervisor. |

Ejemplo: Lanzar con SLAM sin mapa previo:
```bash
ros2 launch surge_et_ambula justina.launch.xml mapless:=true
```

---

## 5. Panel Gráfico de Control y Pruebas Completas (GUI Controller)

El paquete `gui_controller` es una interfaz gráfica desarrollada en PyQt5 diseñada para orquestar y monitorear pruebas completas y misiones del robot. Permite seleccionar una tarea de competencia, lanzar automáticamente `surge_et_ambula` con los parámetros correspondientes, iniciar el `supervisor_manager`, y supervisar en tiempo real que todos los nodos requeridos estén operando correctamente.

### 5.1 Requisito previo: Definir la variable de entorno
El GUI utiliza la variable `JUSTINA_WS` para resolver la ruta al workspace de Justina y cargar los entornos correspondientes. Asegúrate de tenerla configurada:
```bash
export JUSTINA_WS="$HOME/Justina"
```
Para dejarla persistente, agrégala a tu `~/.bashrc`:
```bash
echo 'export JUSTINA_WS="$HOME/Justina"' >> ~/.bashrc
source ~/.bashrc
```

### 5.2 Lanzar el Panel de Control
En una terminal con el workspace fuenteado:
```bash
source install/setup.bash
ros2 run gui_controller gui_controller
```

### 5.3 Funcionalidades y Flujo de Operación

1. **Limpieza automática de nodos huérfanos:**
   Al iniciar, el panel revisa y cierra procesos huérfanos que hayan quedado de ejecuciones anteriores para evitar colisiones de nombres o puertos.

2. **Selección del Perfil de Planeación (Planning Profiles):**
   En el menú desplegable puedes seleccionar la prueba a ejecutar:
   * **HRI:** Tarea de bienvenida e interacción humano-robot.
   * **PNP Justina:** Manipulación y Pick and Place con los brazos de Justina.
   * **PNP Takeshi:** Tarea de Pick and Place adaptada para la plataforma Takeshi.
   * **Laundry:** Tarea de manejo de ropa / lavandería.
   * **Restaurant:** Tarea de servicio en restaurante (configura automáticamente `plan_name:=restaurant mapless:=true`).
   * **Robot Inspection:** Rutina de inspección de estado del robot.
   * **GPSR:** General Purpose Service Robot (configura automáticamente `plan_name:=goals` y activa la planeación por lenguaje natural en el supervisor).

3. **Lanzamiento de Surge et Ambula ("Start Surge"):**
   Al presionar el botón **Start Surge**, el panel abre una terminal independiente que exporta las variables de CycloneDDS, ejecuta `surge_et_ambula justina.launch.xml` con los argumentos del plan seleccionado y arranca la arquitectura base.

4. **Monitoreo de Nodos en Tiempo Real:**
   El panel central contiene la lista de los 69 nodos esperados definidos en `nodes.yaml`. Conforme los nodos arrancan, su estado cambia automáticamente a activo (verde). Si algún nodo falla o se apaga, se refleja de inmediato en la lista.

5. **Inicio de la Misión con el Supervisor ("Start Supervisor"):**
   Una vez que los nodos de Surge están arriba y en verde, presionar **Start Supervisor** inicia el nodo `supervisor_manager_node` para comenzar la ejecución secuencial de la tarea.

---

## 6. Pruebas Básicas y Monitoreo

### 6.1 Comprobar nodos activos
```bash
ros2 node list
```

### 6.2 Probar movimiento de la base (Teleoperación)
Para verificar la tracción omnidireccional:
```bash
ros2 run teleop_twist_keyboard teleop_twist_keyboard --ros-args --remap cmd_vel:=/hardware/base/cmd_vel
```

O enviar una velocidad puntual (0.1 m/s hacia adelante):
```bash
ros2 topic pub --once /hardware/base/cmd_vel geometry_msgs/msg/Twist "{linear: {x: 0.1, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.0}}"
```

### 6.3 Monitorear sensores
```bash
# Ver odometría en tiempo real
ros2 topic echo /hardware/base/odom

# Ver lecturas del láser Hokuyo
ros2 topic echo /hardware/base/scan --field ranges

# Ver el estado de todos los servomotores / articulaciones
ros2 topic echo /hardware/joint_states
```

---

## 7. Ejecución de Servicios (Services)

Puedes probar individualmente cada módulo del robot mediante llamadas de servicio desde terminal (`ros2 service call`). Todos los servicios y parámetros a continuación han sido verificados directamente en el código fuente:

### HRI (Interacción Humano-Robot)
* **Texto a Voz (TTS - Talker):**
  ```bash
  ros2 service call /hri/talker/talk talker_interfaces/srv/Talk "{text: 'Hola, soy Justina'}"
  ```
* **Reconocimiento de Voz (STT - Listener):**
  ```bash
  ros2 service call /hri/listener/listen listener_interfaces/srv/Listen "{enable: true, grammar: ''}"
  ```
* **Activar/Desactivar Seguimiento de Personas (Human Follower):**
  ```bash
  # Activar
  ros2 service call /hri/human_tracking/enable std_srvs/srv/SetBool "{data: true}"
  # Desactivar
  ros2 service call /hri/human_tracking/enable std_srvs/srv/SetBool "{data: false}"
  ```
* **Detectar persona señalando, saludando o levantando la mano (Wrist Detection):**
  > Requiere `action_type` (`'pointing'`, `'waving'`, o `'raising'`) y `arm_side` (`'any'`, `'left'`, o `'right'`):
  ```bash
  ros2 service call /human_detection/detect_wrist human_interfaces/srv/DetectPointing "{action_type: 'pointing', arm_side: 'any'}"
  ```

### Hardware y Actuadores
* **Grippers (Debe especificarse izquierdo o derecho):**
  ```bash
  # Abrir / Cerrar Gripper Izquierdo ('open', 'close', 'stop', 'query_status')
  ros2 service call /hardware/grippers/move_left_gripper grippers_interfaces/srv/MoveGrippers "{cmd: 'open'}"
  ros2 service call /hardware/grippers/move_left_gripper grippers_interfaces/srv/MoveGrippers "{cmd: 'close'}"

  # Abrir / Cerrar Gripper Derecho
  ros2 service call /hardware/grippers/move_right_gripper grippers_interfaces/srv/MoveGrippers "{cmd: 'open'}"
  ros2 service call /hardware/grippers/move_right_gripper grippers_interfaces/srv/MoveGrippers "{cmd: 'close'}"
  ```
* **Brazos (Brazo izquierdo y derecho por separado — cada brazo tiene 6 grados de libertad):**
  ```bash
  # Mover brazo derecho (6 ángulos en radianes: hombro_f, hombro_s, twist, codo, muñeca_rot, muñeca_flex y velocidad)
  ros2 service call /hardware/arms/move_right_arm arms_interfaces/srv/MoveArms "{positions: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0], speed: 10}"

  # Mover brazo izquierdo (6 ángulos en radianes)
  ros2 service call /hardware/arms/move_left_arm arms_interfaces/srv/MoveArms "{positions: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0], speed: 10}"
  ```
* **Mover la Cabeza (Pan y Tilt en radianes — requiere 2 valores):**
  ```bash
  # [pan, tilt] -> 0.0 al frente, -0.3 inclinada abajo
  ros2 service call /hardware/head/move_head head_interfaces/srv/MoveHead "{positions: [0.0, -0.3]}"
  ```
* **Mover el Torso (Elevación en metros, rango 0.0 a 0.30 m):**
  ```bash
  ros2 service call /hardware/torso/move_torso torso_interfaces/srv/MoveTorso "{position: 0.25}"
  ```
* **Sensor de Puerta (Door Detector):**
  ```bash
  ros2 service call /hardware/door_detector/check std_srvs/srv/Trigger "{}"
  ```

### Navegación y Gestor de Locaciones
* **Navegar a una ubicación conocida:**
  ```bash
  ros2 service call /nav_controller/go_to_location known_locations_interfaces/srv/LocationsGet "{location_name: 'kitchen'}"
  ```
* **Guardar la pose actual del robot (usando TF automático):**
  ```bash
  ros2 service call /locations_manager/add known_locations_interfaces/srv/LocationsServer "{location_name: 'entrance'}"
  ```
* **Guardar una pose específica manualmente:**
  ```bash
  ros2 service call /locations_manager/save known_locations_interfaces/srv/SavePoseLocation "{location_name: 'table_1', pose: {position: {x: 1.2, y: 0.5, z: 0.0}, orientation: {x: 0.0, y: 0.0, z: 0.0, w: 1.0}}}"
  ```
* **Consultar las coordenadas de una ubicación guardada:**
  ```bash
  ros2 service call /locations_manager/get known_locations_interfaces/srv/LocationsGet "{location_name: 'kitchen'}"
  ```

### Visión y Percepción
* **Detección y Pose 3D por Clase (Detect Fusion - YOLO u OWLv2):**
  > **Importante:** El modelo debe especificarse en MAYÚSCULAS (`'YOLO'` u `'OWL'`):
  ```bash
  ros2 service call /detect_by_class vision_interfaces/srv/DetectByClass "{model: 'YOLO', class_name: 'bottle'}"
  ```
* **Reconocimiento Facial (Face Florence 2):**
  ```bash
  # Detectar rostro en la imagen actual
  ros2 service call /vision/detect_person faces_interfaces/srv/FaceRecog "{name: ''}"

  # Registrar persona con nombre
  ros2 service call /vision/register_person faces_interfaces/srv/FaceRecog "{name: 'John'}"

  # Identificar persona contra rostros conocidos
  ros2 service call /vision/identify_person faces_interfaces/srv/FaceRecog "{name: ''}"

  # Describir atributos de la persona
  ros2 service call /vision/describe_person faces_interfaces/srv/FaceRecog "{name: ''}"
  ```
* **Alineación con mesa:**
  ```bash
  ros2 service call /vision/get_table_alignment vision_interfaces/srv/GetTableAlignment "{is_round: false}"
  ```
* **Buscar asiento disponible (Find Seat):**
  ```bash
  ros2 service call /vision/find_seat vision_interfaces/srv/FindSeat "{}"
  ```

### Conocimiento (Knowledge)
* **Obtener el estado del mundo (`GetWorldState`):**
  > Categorías disponibles: `'world'` (todo el modelo del mundo), `'known_locations'`, `'robot'`, o una clase como `'drink'`:
  ```bash
  # Consultar el estado completo del mundo en formato JSON
  ros2 service call /knowledge/knowledge_manager/world/get_state knowledge_interfaces/srv/GetWorldState "{category: 'world', id: '', attribute: ''}"

  # Consultar locaciones conocidas
  ros2 service call /knowledge/knowledge_manager/world/get_state knowledge_interfaces/srv/GetWorldState "{category: 'known_locations', id: '', attribute: ''}"
  ```
* **Consultar frases del robot:**
  ```bash
  ros2 service call /knowledge/knowledge_manager/phrases/get_phrase knowledge_interfaces/srv/GetPhrase "{phrase: 'start_hri', context: 'hri'}"
  ```

### Planeación Simbólica (Planning y CLIPS)
* **Parser Semántico (Lenguaje natural a primitivas CD):**
  ```bash
  ros2 service call /planning/semantic_parser/semantic_parser semantic_parser_interfaces/srv/Parser "{command: 'go to the kitchen and take the apple'}"
  ```
* **Extraer entidades desde lenguaje natural:**
  ```bash
  ros2 service call /planning/semantic_parser/extract_entity semantic_parser_interfaces/srv/ExtractEntity "{sentence: 'my name is John and I want water', category: 'names'}"
  ```
* **Iniciar ciclo de planeación CLIPS:**
  ```bash
  ros2 service call /planning/clips/start_planning std_srvs/srv/Trigger "{}"
  ```

---

## 8. Ejecución de Acciones (Actions con Executor Worker)

El sistema de ejecución de tareas de alto nivel se gestiona mediante la acción `/executor/execute_task` (`executor_interfaces/action/ExecuteTask`).

### 8.1 Sintaxis General
```bash
ros2 action send_goal /executor/execute_task executor_interfaces/action/ExecuteTask \
  "{task: {step_id: 1, agent: 'Justina', action: '<ACCION>', args: ['<ARGUMENTO>']}}"
```

### 8.2 Catálogo de Acciones Disponibles en el Worker

| Acción (`action`) | Ejemplo de Argumento (`args`) | Descripción |
|---|---|---|
| `say` | `["Hello, I am ready to help you"]` | Habla la frase indicada por TTS. |
| `listen` | `[""]` | Escucha al operador y extrae la instrucción. |
| `go_to` | `["kitchen"]` o `["table_1"]` | Navega a la ubicación especificada. |
| `find_person` | `["operator"]` | Busca e identifica a una persona. |
| `find_object` | `["apple"]` o `["cup"]` | Busca el objeto en el campo de visión / mesa. |
| `take` | `["apple"]` | Planea y ejecuta agarre fino (`VisualGraspApproach`). |
| `drop` | `["table"]` | Suelta o coloca el objeto en la superficie destino. |
| `align` | `["table"]` | Alinea la base del robot con respecto a la mesa. |
| `approach` | `["table"]` | Se aproxima a la distancia de agarre de la mesa. |
| `follow` | `["start"]` o `["stop"]` | Inicia o detiene el seguimiento de la persona. |
| `point` | `["kitchen"]` | Apunta con el brazo indicando una dirección. |
| `give` | `["object"]` | Entrega un objeto en la mano del usuario. |
| `receive` | `["object"]` | Recibe un objeto que el usuario coloca en el gripper. |
| `check` | `["door"]` | Verifica el estado del sensor de puerta. |
| `fold` | `["arms"]` | Pliega los brazos a su posición segura de navegación. |
| `save_loc` | `["custom_point"]` | Guarda la ubicación actual en el mapa. |

#### Ejemplo práctico: Enviar a Justina a decir una frase mediante Action
```bash
ros2 action send_goal /executor/execute_task executor_interfaces/action/ExecuteTask \
  "{task: {step_id: 1, agent: 'Justina', action: 'say', args: ['Iniciando protocolo de prueba']}}"
```

#### Ejemplo práctico: Enviar a Justina a navegar a la cocina mediante Action
```bash
ros2 action send_goal /executor/execute_task executor_interfaces/action/ExecuteTask \
  "{task: {step_id: 2, agent: 'Justina', action: 'go_to', args: ['kitchen']}}"
```

---

## 9. Catálogo de Tópicos Principales

| Tópico | Tipo de Mensaje | Descripción |
|---|---|---|
| `/hardware/base/cmd_vel` | `geometry_msgs/msg/Twist` | Velocidades lineal y angular enviadas a la base omnidireccional. |
| `/hardware/base/odom` | `nav_msgs/msg/Odometry` | Odometría calculada por los encoders de Robotino. |
| `/hardware/base/scan` | `sensor_msgs/msg/LaserScan` | Datos del escáner láser 2D Hokuyo frontal. |
| `/hardware/joint_states` | `sensor_msgs/msg/JointState` | Posiciones, velocidades y esfuerzos de todas las articulaciones. |
| `/hardware/camera/image_raw` | `sensor_msgs/msg/Image` | Imagen RGB de la cámara montada en la cabeza (Kinect/Xtion). |
| `/hardware/camera/points` | `sensor_msgs/msg/PointCloud2` | Nube de puntos 3D generada por la cámara de profundidad. |
| `/navigation/map` | `nav_msgs/msg/OccupancyGrid` | Mapa de ocupación 2D para localización y costmaps. |
| `/navigation/amcl_pose` | `geometry_msgs/msg/PoseWithCovarianceStamped` | Estimación de la pose del robot según AMCL. |
| `/vision/yolo_detections` | `vision_interfaces/msg/YoloDetections` | Resultados de detección y clasificación de objetos 2D/3D. |
| `/hri/human_tracking/tracks` | `human_interfaces/msg/PersonTracks` | Poses y tracking de personas detectadas. |
| `/supervisor/system_status` | `supervisor_interfaces/msg/SystemStatus` | Estado de salud y actividad de los módulos del sistema. |

---

## 10. Modo Simulación

Si no dispones del robot físico o quieres realizar pruebas de lógica, planeación o navegación de forma aislada, puedes lanzar el entorno simulado:

```bash
ros2 launch surge_et_ambula justina_sim.launch.xml
```

Este launch incluye:
* Simulación cinemática de la base omnidireccional (`simple_justina_sim.py`).
* Publicador de TF y modelo URDF completo (`robot_state_publisher`).
* Generación de láser simulado contra el mapa de navegación (`/hardware/base/scan`).
* Módulos completos de Supervisor, Executor, Planning, Navegación y RViz2.

Para deshabilitar módulos pesados durante pruebas de navegación en simulación:
```bash
ros2 launch surge_et_ambula justina_sim.launch.xml use_manipulation:=false use_vision:=false
```
