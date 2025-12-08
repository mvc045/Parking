
cmake_minimum_required(VERSION 3.10)

project(Parking VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Где искать заголовочные файлы (.hpp)
include_directories(include)

# Собираем список всех .cpp файлов из папки src
file(GLOB SOURCES "src/*.cpp")

# Создаем исполняемый файл (Executable)
add_executable(Parking ${SOURCES})

find_package(Threads REQUIRED)
target_link_libraries(Parking Threads::Threads)
