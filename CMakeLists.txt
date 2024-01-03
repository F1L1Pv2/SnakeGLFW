# CMakeList.txt : CMake project for SnakeGLFW, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("SnakeGLFW")



include_directories(${CMAKE_SOURCE_DIR}/dependencies/include)
include_directories(${CMAKE_SOURCE_DIR}/src)
link_directories(${CMAKE_SOURCE_DIR}/dependencies/libs)

# Add source to this project's executable.
add_executable (SnakeGLFW "src/entry.cpp" "dependencies/src/glad.c")

# Assets
add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
)
add_dependencies(SnakeGLFW copy_assets)

# Dependencies
target_link_libraries(SnakeGLFW glfw3)
target_link_libraries(SnakeGLFW opengl32)

set_target_properties(SnakeGLFW PROPERTIES 
    LINK_FLAGS "/ENTRY:mainCRTStartup /SUBSYSTEM:WINDOWS")



if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET SnakeGLFW PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
