cmake_minimum_required(VERSION 3.16.1)
project(Robotproject)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

# 添加编译选项
# if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
#     add_compile_options(-Wall -Wextra -Wpedantic)
# endif()

# 添加 pthread 支持
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_BUILD_TYPE "Debug")

# 查找当前目录下的所有源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_NAME}_src)

add_subdirectory(src/sensors)
add_subdirectory(src/detect)
add_subdirectory(src/serialport)
add_subdirectory(src/record)
add_subdirectory(src/utility)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_src})

target_link_libraries(${PROJECT_NAME} PRIVATE
        sensors
        detect
        serialport
        record
        utility
        ${OpenCV_LIBRARIES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${OpenCV_INCLUDE_DIRS}
        src/sensors/include
        src/detect/include
        src/serialport/include
        src/record/include
        src/utility/include
        ./config
)