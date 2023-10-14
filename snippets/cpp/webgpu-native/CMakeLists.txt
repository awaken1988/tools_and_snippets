cmake_minimum_required(VERSION 3.20.0)
project(webgpu_test VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_subdirectory(extern_dawn)
add_subdirectory(extern_glfw3webgpu)

add_executable(webgpu_hello 
    src/main.cpp
)
target_include_directories(webgpu_hello PUBLIC ./src)
target_link_libraries(webgpu_hello PUBLIC webgpu glfw glfw3webgpu)