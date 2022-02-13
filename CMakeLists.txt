cmake_minimum_required(VERSION 3.14)

project(
    "arglet"
    VERSION 0.0.0
    DESCRIPTION "Declarative Command Line Argument Parser for C++"
    HOMEPAGE_URL "https://github.com/codeinred/arglet/"
    LANGUAGES CXX
)

add_library(arglet INTERFACE)
add_library(arglet::arglet ALIAS arglet)

# For ${CMAKE_INSTALL_<DIR>} variables that are standarized
include(GNUInstallDirs)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

target_compile_features(
    arglet
    INTERFACE
    cxx_std_20
)

target_include_directories(
    arglet
    INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Installation
# See: https://dominikberner.ch/cmake-interface-lib/

install(
    TARGETS arglet
    EXPORT ${PROJECT_NAME}_Targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/arglet/cmake
)

install(
    EXPORT ${PROJECT_NAME}_Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE arglet::
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/arglet/cmake
)

install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
              "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/arglet/cmake
)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/arglet
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)


include(helper)

find_or_fetch(
        tuplet
        https://github.com/codeinred/tuplet.git
        main)
target_link_libraries(arglet INTERFACE tuplet::tuplet)
if(PROJECT_IS_TOP_LEVEL)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)

    find_or_fetch(
        fmt
        https://github.com/fmtlib/fmt.git
        master)

    find_or_fetch(
        Catch2
        https://github.com/catchorg/catch2.git
        devel
        3.0.0)

    FetchContent_MakeAvailable(${remote_dependencies})

    add_executable(test_arglet
        test/test_arglet.cpp)
    target_link_libraries(test_arglet PRIVATE
        arglet::arglet
        fmt::fmt
        Catch2::Catch2WithMain)
    add_source_dir(
        examples # the name of the directory
        arglet::arglet # Libraries to link against
        fmt::fmt)

    # Append places where Catch2 might be to the CMake Module path, so we can
    # use the function catch_discover_tests
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_INSTALL_PREFIX}/lib/cmake/Catch2)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    include(CTest)
    include(Catch)
    catch_discover_tests(test_arglet)
endif()


