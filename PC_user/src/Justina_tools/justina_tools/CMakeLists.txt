cmake_minimum_required(VERSION 3.5)
project(justina_tools)

set(CMAKE_CXX_STANDARD 14)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(robotino_msgs REQUIRED)
find_package(nlohmann_json REQUIRED)

include_directories(
  include
  ${rclcpp_INCLUDE_DIRS}
  ${std_msgs_INCLUDE_DIRS}
  ${geometry_msgs_INCLUDE_DIRS}
  ${robotino_msgs_INCLUDE_DIRS}
)

add_library(justina_tools
  src/JustinaHRI.cpp
  src/JustinaHardware.cpp
  src/JustinaNavigation.cpp
  src/JustinaVision.cpp
)

ament_target_dependencies(justina_tools
  rclcpp
  std_msgs
  geometry_msgs
  robotino_msgs
  nlohmann_json
)

install(DIRECTORY include/
  DESTINATION include
)

install(TARGETS justina_tools
  DESTINATION lib/${PROJECT_NAME}
)

ament_export_include_directories(include)
ament_export_libraries(justina_tools)

ament_package()

