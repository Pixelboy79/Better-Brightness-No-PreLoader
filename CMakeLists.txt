cmake_minimum_required(VERSION 3.14)
project(BetterBrightness)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Fetch Dobby directly from GitHub
include(FetchContent)
FetchContent_Declare(
    dobby
    GIT_REPOSITORY https://github.com/jmpews/Dobby.git
    GIT_TAG        master 
)
FetchContent_MakeAvailable(dobby)

# 2. Find the Android logging library (for LOGI and LOGE)
find_library(log-lib log)

# 3. Create your BetterBrightness .so file
add_library(BetterBrightness SHARED src/main.cpp)

# 4. Include directories (so CMake can find dobby.h)
target_include_directories(BetterBrightness PRIVATE 
    ${dobby_SOURCE_DIR}/include
)

# 5. Link Dobby and the log library to your mod
target_link_libraries(BetterBrightness
    PRIVATE
    dobby
    ${log-lib}
)
