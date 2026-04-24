cmake_minimum_required(VERSION 3.22.1)
project(BetterBrightness)

# Completely removed Preloader dependencies
add_library(BetterBrightness SHARED src/main.cpp)

target_compile_features(BetterBrightness PRIVATE cxx_std_20)
target_link_libraries(BetterBrightness log)
