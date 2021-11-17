cmake_minimum_required(VERSION 3.11)

project(Museu)

add_executable(${PROJECT_NAME} main.cpp openglwindow.cpp camera.cpp model.cpp)

enable_abcg(${PROJECT_NAME})
