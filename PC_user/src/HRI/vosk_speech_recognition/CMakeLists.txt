cmake_minimum_required(VERSION 3.8)
project(vosk_speech_recognition)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
find_package(std_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclpy REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "srv/SpeechRecog.srv"
  DEPENDENCIES builtin_interfaces std_msgs
)

ament_export_dependencies(rosidl_default_runtime)

# Instalar el nodo Python
install(PROGRAMS
  src/speech_recognizer.py
  DESTINATION lib/${PROJECT_NAME}
)

# Instalar el modelo de Vosk
install(
  DIRECTORY model/
  DESTINATION share/${PROJECT_NAME}/model
)

# Instalar la carpeta grammar
install(
  DIRECTORY grammar/
  DESTINATION share/${PROJECT_NAME}/grammar
)

#Instalar archivos lauch
install(
  DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}/launch
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
