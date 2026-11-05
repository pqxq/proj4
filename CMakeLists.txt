cmake_minimum_required(VERSION 3.25)
project(proj4 CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_executable(proj4
    main.cpp
    task7.cpp
)

target_link_libraries(proj4 PRIVATE Threads::Threads)