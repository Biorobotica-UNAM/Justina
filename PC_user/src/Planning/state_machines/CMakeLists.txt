cmake_minimum_required(VERSION 3.5)
project(state_machines)

set(CMAKE_CXX_STANDARD 14)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(robotino_msgs REQUIRED)
find_package(vosk_speech_recognition REQUIRED)
find_package(justina_tools REQUIRED)

include_directories(
  include
  ${rclcpp_INCLUDE_DIRS}
  ${std_msgs_INCLUDE_DIRS}
  ${geometry_msgs_INCLUDE_DIRS}
  ${robotino_msgs_INCLUDE_DIRS}
  ${vosk_speech_recognition_INCLUDE_DIRS}
  ${justina_tools_INCLUDE_DIRS}
)

add_executable(robot_inspection src/robot_inspection.cpp)

ament_target_dependencies(robot_inspection
  rclcpp
  std_msgs
  geometry_msgs
  robotino_msgs
  vosk_speech_recognition
  justina_tools
)

install(TARGETS robot_inspection
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()

