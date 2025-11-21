cmake_minimum_required(VERSION 3.10)
project(Lab5_DynamicArray)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Более строгие флаги компиляции
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

# Include headers
include_directories(${PROJECT_SOURCE_DIR}/include)

# Main executable
add_executable(main
    main.cpp
    src/MemoryResource.cpp
)

# GoogleTest
find_package(GTest)
if(GTEST_FOUND)
    message(STATUS "GoogleTest найден, включаем тесты")
    enable_testing()
    
    add_executable(tests
        test/test.cpp
        src/MemoryResource.cpp
    )
    
    target_link_libraries(tests GTest::GTest GTest::Main)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_link_libraries(tests pthread)
    endif()
    
    add_test(NAME unit_tests COMMAND tests)
else()
    message(WARNING "GoogleTest не найден, тесты отключены")
endif()