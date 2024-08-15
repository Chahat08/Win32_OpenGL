cmake_minimum_required(VERSION 3.10)

project(Win32_OpenGL_App)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories("C:/dev/opengl/include")

add_executable(Win32_OpenGL_App WIN32 main.cpp "glad.c")

include_directories("C:/dev/opengl/lib")

target_link_libraries(Win32_OpenGL_App opengl32 gdi32)
