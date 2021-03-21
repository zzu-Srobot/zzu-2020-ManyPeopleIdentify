cmake_minimum_required(VERSION 3.0.2)
project(multi_recognation)

add_compile_options(-std=c++11)
find_package(catkin REQUIRED COMPONENTS
	roscpp
	std_msgs
	genmsg
	actionlib_msgs
	actionlib
	rostime 
	sensor_msgs 
	message_filters 
	cv_bridge 
	image_transport 
	compressed_image_transport 
	compressed_depth_image_transport 
	kinect2_bridge
)

find_package(OpenCV REQUIRED)
find_package(OpenMP)
find_package(PCL REQUIRED)

catkin_package()

if(OPENMP_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

include_directories(
~/catkin_ws/include
${catkin_INCLUDE_DIRS}
${OpenCV_INCLUDE_DIRS}
${PCL_INCLUDE_DIRS}
${kinect2_bridge_INCLUDE_DIRS}
)
LINK_DIRECTORIES(/home/robot/catkin_ws/lib/)
add_executable(multi_recognation src/multi_recognation.cpp)
add_dependencies(multi_recognation ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(multi_recognation
  ${OpenCV_LIBRARIES}
  ${PCL_LIBRARIES}
  ${kinect2_bridge_LIBRARIES}
  ${catkin_LIBRARIES} -lcurl -ljsoncpp
  ${catkin_LIBRARIES} libmsc.so -ldl -pthread
  ${catkin_LIBRARIES} libasound.so -ldl -pthread
${catkin_LIBRARIES} ~/catkin_ws/lib/libSeetaFaceCropper200.so -ldl -pthread
  ${catkin_LIBRARIES} ~/catkin_ws/lib/libSeetaFaceDetector200.so -ldl -pthread
  ${catkin_LIBRARIES} ~/catkin_ws/lib/libSeetaFaceRecognizer200.so -ldl -pthread
  ${catkin_LIBRARIES} ~/catkin_ws/lib/libSeetaPointDetector200.so -ldl -pthread
)
add_executable(multi_recognation2 src/multi_recognation2.cpp)
add_dependencies(multi_recognation2 ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(multi_recognation2
  ${OpenCV_LIBRARIES}
  ${PCL_LIBRARIES}
  ${kinect2_bridge_LIBRARIES}
  ${catkin_LIBRARIES} -lcurl -ljsoncpp
  ${catkin_LIBRARIES} libmsc.so -ldl -pthread
  ${catkin_LIBRARIES} libasound.so -ldl -pthread
${catkin_LIBRARIES} ~/catkin_ws/lib/libSeetaFaceCropper200.so -ldl -pthread
  ${catkin_LIBRARIES} ~/catkin_ws/lib/libSeetaFaceDetector200.so -ldl -pthread
  ${catkin_LIBRARIES} ~/catkin_ws/lib/libSeetaFaceRecognizer200.so -ldl -pthread
  ${catkin_LIBRARIES} ~/catkin_ws/lib/libSeetaPointDetector200.so -ldl -pthread
)


