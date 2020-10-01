cmake_minimum_required(VERSION 3.0.0)
project(arglet CXX)

add_library(arglet INTERFACE)

option(arglet_Install "Install CMake targets during install step." ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (MSVC)
    target_compile_options(arglet INTERFACE "/std:c++latest" "/await" "/EHsc")
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
    target_Compile_options(arglet INTERFACE "-std=c++2a")
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
    target_compile_options(arglet INTERFACE "-std=c++2a")
endif()

target_compile_definitions(arglet INTERFACE)

target_include_directories(
    arglet
    INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>
    $<INSTALL_INTERFACE:include>
)

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    DESTINATION include
)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

    file(GLOB all_examples "examples/*.cpp")

    foreach(file_name ${all_examples})
        get_filename_component(test_name ${file_name} NAME_WE)
        add_executable("${test_name}" "${file_name}")
        target_link_libraries("${test_name}" PRIVATE arglet)
    endforeach()
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)

    file(GLOB all_tests "test/*.cpp")

    foreach(file_name ${all_tests})
        get_filename_component(test_name ${file_name} NAME_WE)
        add_executable("${test_name}" "${file_name}")
        target_link_libraries("${test_name}" PRIVATE arglet)
        add_test(NAME "${test_name}" COMMAND "${test_name}")
    endforeach()
endif()


