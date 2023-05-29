cmake_minimum_required(VERSION 3.20.0)
project(vulkan_example VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(hello 
    main.cpp
    hello.h
    extern/stb_image.h
)

IF(WIN32)
    #Usage:
    #   Install: LunarG Vulkan SDK, GLFW
    #   example: cmake ../ -DEXTERN_LUNARG="C:/VulkanSDK/1.3.243.0/" -DEXTERN_GLFW="C:/glfw-3.3.8.bin.WIN64"

    set(EXTERN_GLM "${EXTERN_LUNARG}/Include/glm")

    message(STATUS "add GLM from ${EXTERN_GLM}")
    add_subdirectory(${EXTERN_GLM} ${CMAKE_CURRENT_BINARY_DIR}/glm)

    target_include_directories(hello PUBLIC "${EXTERN_LUNARG}/Include")
    target_include_directories(hello PUBLIC "${EXTERN_GLFW}/include")

    target_link_options(hello PUBLIC vulkan-1.lib  /LIBPATH:${EXTERN_LUNARG}/Lib)
	target_link_options(hello PUBLIC glfw3.lib  /LIBPATH:${EXTERN_GLFW}/lib-vc2022)
ELSE()
    find_package(glfw3 3.3 REQUIRED)
    find_package(glm REQUIRED)
    find_package(Vulkan REQUIRED)
    target_link_libraries(hello PRIVATE glfw glm vulkan)
ENDIF()

target_link_libraries(hello PRIVATE glm)

#glslc shader.vert -o vert.spv