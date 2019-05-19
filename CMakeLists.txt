cmake_minimum_required(VERSION 2.8.3)
project(keyboard_tf_tuner)

## Compile as C++11, supported in ROS Kinetic and newer
add_compile_options(-std=c++11)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  std_msgs
  tf
  tf2
)

catkin_package(
#  INCLUDE_DIRS include
#  LIBRARIES temoto_tests
#  CATKIN_DEPENDS roscpp rospy std_msgs
#  DEPENDS system_lib
)

include_directories( ${catkin_INCLUDE_DIRS})

# transform tuner
add_executable(tuner src/keyboard_tf_tuner.cpp)
add_dependencies(tuner ${catkin_EXPORTED_TARGETS} ${${PROJECT_NAME}_EXPORTED_TARGETS})
target_link_libraries(tuner ${catkin_LIBRARIES} )
