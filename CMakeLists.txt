cmake_minimum_required(VERSION 3.10)

# Set the project name
project(Win32_OpenGL_App)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
add_executable(Win32_OpenGL_App WIN32 main.cpp)

# Link against the OpenGL and GDI32 libraries
target_link_libraries(Win32_OpenGL_App opengl32 gdi32)
