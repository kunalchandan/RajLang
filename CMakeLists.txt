cmake_minimum_required(VERSION 3.10)
project(raj)

# Set C++ version
set(CMAKE_CXX_STANDARD 17)

# Set source files
set(SOURCES
    src/raj.cpp
    src/Lexer.cpp
)

# Set header files
set(HEADERS
    src/Lexer.hpp
)


set(TEST_SOURCES
    test/test_main.cpp
)

# Create test executable
add_executable(rajTests ${TEST_SOURCES})

# Create executable
add_executable(raj ${SOURCES} ${HEADERS})

# Set include directories
target_include_directories(raj
    PRIVATE include/Catch/single_include/
    PRIVATE include/CLI11/include/
    PRIVATE include/magic_enum/include/

)
# Set include directories
target_include_directories(rajTests PRIVATE include)