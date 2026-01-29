cmake_minimum_required(VERSION 3.20)
project(glint VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SQLite3 REQUIRED)
find_package(Curses REQUIRED)

add_executable(glint
    src/main.cpp
    src/crawler.cpp
    src/database.cpp
)

target_include_directories(glint PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CURSES_INCLUDE_DIRS}
)

target_link_libraries(glint PRIVATE
    SQLite::SQLite3
    ${CURSES_LIBRARIES}
)

if(WIN32)
    target_link_libraries(glint PRIVATE ws2_32)
endif()
