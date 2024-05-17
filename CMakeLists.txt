cmake_minimum_required(VERSION 3.10)
project(ComputerClub)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Добавляем исполняемый файл и указываем все исходные файлы
add_executable(task main.cpp club.cpp)

# Указываем путь к заголовочным файлам
target_include_directories(task PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})