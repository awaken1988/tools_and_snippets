cmake_minimum_required(VERSION 3.20.0)
project(blocks VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(bgfx.cmake)

add_executable(blocks 
    main.cpp
)

IF(WIN32)
	target_link_options(blocks PUBLIC glfw3.lib /LIBPATH:${EXTERN_GLFW}/lib-vc2022)
    target_include_directories(blocks PUBLIC "${EXTERN_GLFW}/include")
ELSE()
    find_package(glfw3 3.3 REQUIRED)
ENDIF()

target_include_directories(blocks PUBLIC "${EXTERN_GLFW}/include")
target_link_libraries(blocks PUBLIC bgfx bx)