cmake_minimum_required(VERSION 3.14)
project(BetterBrightness)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the Android logging library
find_library(log-lib log)

# Create your BetterBrightness .so file
add_library(BetterBrightness SHARED src/main.cpp)

# Link the log library
target_link_libraries(BetterBrightness PRIVATE ${log-lib})
