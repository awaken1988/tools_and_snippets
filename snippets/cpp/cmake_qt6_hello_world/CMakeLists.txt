#Problem: missing qt dlls while debugging:
#   - TODO: https://gitlab.kitware.com/cmake/cmake/-/issues/8884
#   - does not work: set(CMAKE_MSVCIDE_RUN_PATH "C:\\Qt\\6.0.2\\msvc2019_64\\bin")

#Additional PATH needed in PATH variable:
#   -   e.g C:\Qt\6.0.2\msvc2019_64\bin                               //need by Visual Studio debugger to find the Qt6 dlls
#   -   e.g C:\Qt\6.0.2\msvc2019_64\lib\cmake\Qt6                     //need by qmake to find Qt6

cmake_minimum_required(VERSION 3.20.0)
project(helloworld VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Qt6 stuff
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
    find_package(Qt6 COMPONENTS Widgets REQUIRED)

add_executable(helloworld main.cpp)
target_link_libraries(helloworld PRIVATE Qt6::Widgets)

