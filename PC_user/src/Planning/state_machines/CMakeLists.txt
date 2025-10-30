cmake_minimum_required(VERSION 3.5)
project(state_machines)

set(CMAKE_CXX_STANDARD 14)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(robotino_msgs REQUIRED)
find_package(festino_tools REQUIRED)
find_package(nlohmann_json REQUIRED)

include_directories(
  include
  ${rclcpp_INCLUDE_DIRS}
  ${std_msgs_INCLUDE_DIRS}
  ${geometry_msgs_INCLUDE_DIRS}
  ${robotino_msgs_INCLUDE_DIRS}
  ${festino_tools_INCLUDE_DIRS}
)

add_executable(robot_inspection src/robot_inspection.cpp)
ament_target_dependencies(robot_inspection
  rclcpp
  std_msgs
  geometry_msgs
  robotino_msgs
  festino_tools
  nlohmann_json
)

install(TARGETS
  robot_inspection
  test_mvbase
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()