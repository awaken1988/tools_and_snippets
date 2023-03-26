cmake_minimum_required(VERSION 3.20.0)
project(vulkan_example VERSION 1.0.0 LANGUAGES CXX)

find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(hello 
    main.cpp
    hello.h
)

target_link_libraries(hello PRIVATE glfw glm vulkan)


