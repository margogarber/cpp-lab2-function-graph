cmake_minimum_required(VERSION 3.15)
project(Lab02 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Lab02 WIN32 src/main.cpp)
target_compile_definitions(Lab02 PRIVATE UNICODE _UNICODE)

if(MSVC)
    target_compile_options(Lab02 PRIVATE /utf-8 /W4)
else()
    target_compile_options(Lab02 PRIVATE -Wall -Wextra)
    target_link_options(Lab02 PRIVATE -municode -static)
endif()
