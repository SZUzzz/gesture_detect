cmake_minimum_required(VERSION 3.0)
project(utility)


find_package(OpenCV REQUIRED)
file(GLOB_RECURSE ${PROJECT_NAME}_src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
)

add_library(${PROJECT_NAME} OBJECT ${${PROJECT_NAME}_src})
target_link_libraries(${PROJECT_NAME} PRIVATE
        ${OpenCV_LIBS}
)
target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)