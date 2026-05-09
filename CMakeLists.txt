cmake_minimum_required(VERSION 3.14)
project(BetterBrightness)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_library(log-lib log)
add_library(BetterBrightness SHARED src/main.cpp)
target_link_libraries(BetterBrightness PRIVATE ${log-lib})
