cmake_minimum_required(VERSION 3.20.0)
project(vulkan_example VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VUL_PREFIX ./src/render/vulkan)
set(ENGINE_PREFIX ./src/engine)
set(EXAMPLE_PREFIX ./src/example)

set(ENGINE_FILES
    ${ENGINE_PREFIX}/EngineTypes.h 
    ${ENGINE_PREFIX}/EngineRender.h
    ${ENGINE_PREFIX}/EngineRender.cpp
)

set(VUL_FILES 
    ${VUL_PREFIX}/VulBase.h
    ${VUL_PREFIX}/VulBase.cpp
    ${VUL_PREFIX}/VulDevice.h
    ${VUL_PREFIX}/VulDevice.cpp
    ${VUL_PREFIX}/VulRender.h
    ${VUL_PREFIX}/VulRender.cpp
)

set(EXAMPLE_FILES
    ${EXAMPLE_PREFIX}/blocks/blocks.h
    ${EXAMPLE_PREFIX}/blocks/blocks.cpp
    ${EXAMPLE_PREFIX}/blocks/blocksTypes.h 
)

set(EXTERNAL_FILES
    src/extern/stb_image/stb_image.h
)


add_executable(hello 
    src/main.cpp
    ${ENGINE_FILES}
    ${VUL_FILES}
    ${EXTERNAL_FILES}
    ${EXAMPLE_FILES}
)

target_include_directories(hello PUBLIC ./src)
target_include_directories(hello PUBLIC ./src/engine)

source_group("engine" ${ENGINE_FILES})
source_group("vulkan" ${VUL_FILES})
source_group("external" ${EXTERNAL_FILES})
source_group("example" ${EXAMPLE_FILES})

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

#Help:
#   For eclipse:
#       cmake -G "Eclipse CDT4 - Unix Makefiles" -B ../build_eclipse -DCMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE -DCMAKE_BUILD_TYPE=Debug