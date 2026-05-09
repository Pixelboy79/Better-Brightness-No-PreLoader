cmake_minimum_required(VERSION 3.14)
project(BetterBrightness)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. FIX FOR ANDROID NDK ASM ERROR: Force Position Independent Code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(DOBBY_GENERATE_SHARED OFF CACHE BOOL "" FORCE)

# 2. Fetch Dobby directly from GitHub
include(FetchContent)
FetchContent_Declare(
    dobby
    GIT_REPOSITORY https://github.com/jmpews/Dobby.git
    GIT_TAG        master 
)
FetchContent_MakeAvailable(dobby)

# 3. Find the Android logging library
find_library(log-lib log)

# 4. Create your BetterBrightness .so file
add_library(BetterBrightness SHARED src/main.cpp)

# 5. Include directories (so CMake can find dobby.h)
target_include_directories(BetterBrightness PRIVATE 
    ${dobby_SOURCE_DIR}/include
)

# 6. Link Dobby and the log library
target_link_libraries(BetterBrightness
    PRIVATE
    dobby
    ${log-lib}
)
