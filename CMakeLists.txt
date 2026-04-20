cmake_minimum_required(VERSION 3.16)
project(web_server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(web_server main.cpp
        main.cpp)
target_link_libraries(web_server PRIVATE pthread)
