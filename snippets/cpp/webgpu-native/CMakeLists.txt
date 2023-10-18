cmake_minimum_required(VERSION 3.20.0)
project(webgpu_test VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Note: under the subdir "webgpu_deps" there must be at least
#   - dawn
#   - glfw
#   - glfw3webgpu
set(WEBGPU_DEPS webgpu_deps)

add_subdirectory(${WEBGPU_DEPS}/dawn)
add_subdirectory(${WEBGPU_DEPS}/glfw3webgpu)

add_executable(webgpu_hello 
    src/main.cpp
)
target_include_directories(webgpu_hello PUBLIC ./src)
target_link_libraries(webgpu_hello PUBLIC webgpu glfw glfw3webgpu)



#C:\dev\webgpu_deps