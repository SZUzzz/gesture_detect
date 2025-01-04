cmake_minimum_required(VERSION 3.0)
project(detect)

SET(OPENVINO_PATH "/usr/local/runtime/include")

find_package(OpenCV REQUIRED)
include_directories("/usr/local/onnxruntime-linux-x64-1.12.1/include" )
include_directories("/usr/include/openvino/")
include_directories("/usr/include/openvino/ie/")
file(GLOB_RECURSE ${PROJECT_NAME}_src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
)

add_library(${PROJECT_NAME} OBJECT ${${PROJECT_NAME}_src})
target_link_libraries(${PROJECT_NAME} PRIVATE
        ${OpenCV_LIBS}
        "/usr/local/lib/libonnxruntime.so"
        "/usr/lib/libopenvino.so"
)
target_include_directories(${PROJECT_NAME} PUBLIC
        ${OPENVINO_PATH}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../config
)