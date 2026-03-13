cmake_minimum_required(VERSION 3.10)
project(test)

find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)


add_executable( serialCommTest src/serialCommTest.cpp)
add_executable( serialPortTest src/serialPortTest.cpp)
add_executable( x10_apiTest src/x10_api_test.cpp)
add_executable( crc16Test src/crc16Test.cpp)
add_executable( lowHighBitTest src/lowHighBitTest.cpp)
add_executable( myActuatorTest src/myActuatorTest.cpp)

target_link_libraries(myActuatorTest PRIVATE rmd)
target_link_libraries(lowHighBitTest PRIVATE rmd)
target_link_libraries(crc16Test PRIVATE rmd)
target_link_libraries(serialCommTest PRIVATE rmd)
target_link_libraries(serialPortTest PRIVATE rmd)
target_link_libraries(x10_apiTest PRIVATE rmd)

# ros2
add_executable(joy_to_cmd_node src/ros2_joy_to_cmd_vel.cpp)

target_link_libraries(joy_to_cmd_node
  rclcpp::rclcpp
)
ament_target_dependencies(joy_to_cmd_node
  rclcpp
  std_msgs
  sensor_msgs
  geometry_msgs
)

add_executable(cmd_vel_to_motor_speed_node src/ros2_cmd_vel_to_motor_speed.cpp)

target_link_libraries(cmd_vel_to_motor_speed_node
  rclcpp::rclcpp
)
ament_target_dependencies(cmd_vel_to_motor_speed_node
  rclcpp
  std_msgs
  sensor_msgs
  geometry_msgs
)

add_executable(actuator_controler_node src/ros2_actuator_controler.cpp)

target_link_libraries(actuator_controler_node
  rclcpp::rclcpp
)
ament_target_dependencies(actuator_controler_node
  rclcpp
  std_msgs
  sensor_msgs
  geometry_msgs
)

target_link_libraries(actuator_controler_node
  rmd
)