cmake_minimum_required(VERSION 3.16)
project(tape_sorter VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)

set(LIB_SOURCES
        src/FileTape.cpp
        src/TapeSorter.cpp
)

add_library(tape_sorter_lib STATIC ${LIB_SOURCES})

# Теперь указываем include/ вместо src/
target_include_directories(tape_sorter_lib PUBLIC include)

add_executable(tape_sorter src/main.cpp)
target_link_libraries(tape_sorter PRIVATE tape_sorter_lib)

option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        v1.14.0
    )
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(tape_sorter_tests
            tests/test_main.cpp
            tests/test_tape.cpp
            tests/test_sorter.cpp
    )

    target_link_libraries(tape_sorter_tests
            PRIVATE
            tape_sorter_lib
            GTest::gtest
    )

    include(GoogleTest)
    gtest_discover_tests(tape_sorter_tests)
endif()

install(TARGETS tape_sorter DESTINATION bin)
