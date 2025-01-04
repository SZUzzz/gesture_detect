cmake_minimum_required(VERSION 3.0)
project(sensors)

# if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
#     add_compile_options(-Wall -Wextra -Wpedantic)
# endif()

# 查找库
find_package(OpenCV REQUIRED)

# 查找源文件
file(GLOB_RECURSE ${PROJECT_NAME}_src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
)

# 添加库
add_library(${PROJECT_NAME} OBJECT ${${PROJECT_NAME}_src})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${OpenCV_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE
        ${OpenCV_LIBRARIES}
        "/opt/MVS/lib/64/libMvCameraControl.so"
)