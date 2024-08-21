cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(Win32_OpenGL_App)

find_package(VTK COMPONENTS 
  CommonColor
  CommonCore
  CommonDataModel
  IOImage
  FiltersSources
  InteractionStyle
  RenderingContextOpenGL2
  RenderingCore
  RenderingFreeType
  RenderingGL2PSOpenGL2
  RenderingOpenGL2
  RenderingVolume
  RenderingVolumeOpenGL2
  RenderingExternal
)

if (NOT VTK_FOUND)
  message(FATAL_ERROR "Win32_OpenGL_App: Unable to find the VTK build folder.")
endif()

include_directories("C:/dev/opengl/include")

# Prevent a "command line is too long" failure in Windows.
set(CMAKE_NINJA_FORCE_RESPONSE_FILE "ON" CACHE BOOL "Force Ninja to use response files.")
add_executable(Win32_OpenGL_App WIN32 MACOSX_BUNDLE main.cpp "glad.c")
  target_link_libraries(Win32_OpenGL_App PRIVATE opengl32 gdi32 ${VTK_LIBRARIES}
)
# vtk_module_autoinit is needed
vtk_module_autoinit(
  TARGETS Win32_OpenGL_App
  MODULES ${VTK_LIBRARIES}
)
