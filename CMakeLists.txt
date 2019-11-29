cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)

project(testas)
include_directories(${GLUT_INCLUDE_DIRS} ${GLEW_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS})
add_executable(testas main.cpp shaderLoader.h shadersLoader.cpp)
find_package(OpenGL REQUIRED COMPONENTS OpenGL)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)

add_dependencies(testas OpenGL::OpenGL)

target_link_libraries(testas OpenGL::OpenGL ${GLUT_LIBRARY} ${GLEW_LIBRARIES} ${SDL2_LIBRARIES})
