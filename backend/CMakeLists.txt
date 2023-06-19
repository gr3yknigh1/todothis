cmake_minimum_required(VERSION 3.0)
project(todothis CXX)
enable_language(CXX)

include(ExternalProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(PROJECT_SOURCES_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
set(PROJECT_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/include")
set(PROJECT_THIRDPARTY_DIR "${CMAKE_CURRENT_LIST_DIR}/thirdparty")

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCES_DIR}/*.cpp")
file(GLOB_RECURSE PROJECT_HEADERS CONFIGURE_DEPENDS "${PROJECT_INCLUDE_DIR}/${PROJECT_NAME}/*.hpp")

add_executable(${PROJECT_NAME})

ExternalProject_Add(spdlog
    INSTALL_COMMAND ""
    SOURCE_DIR ${PROJECT_THIRDPARTY_DIR}/spdlog
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DSPDLOG_BUILD_SHARED=false
)

ExternalProject_Add(httplib
    INSTALL_COMMAND ""
    SOURCE_DIR ${PROJECT_THIRDPARTY_DIR}/cpp-httplib
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
)

add_dependencies(${PROJECT_NAME} spdlog)
add_dependencies(${PROJECT_NAME} httplib)

target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME}
    PRIVATE ${PROJECT_INCLUDE_DIR}
    PRIVATE ${PROJECT_THIRDPARTY_DIR}/spdlog/include
    PRIVATE ${PROJECT_THIRDPARTY_DIR}/cpp-httplib
    )

# TODO: Add release configuration
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -Wall -Werror -Wextra -pedantic -std=c++20")

find_program(CLANG_TIDY_EXEC clang-tidy)
if(CLANG_TIDY_EXEC)
    message(STATUS "Found clang-tidy")
    add_custom_target(
        tidy
        ${CLANG_TIDY_EXEC} ${PROJECT_SOURCES} ${PROJECT_HEADERS} --config-file="${PROJECT_SOURCE_DIR}/.clang-tidy" --use-color -quiet -- ${CMAKE_CXX_FLAGS} -I${PROJECT_INCLUDE_DIR}
        )

else()
    message(WARNING "clang-tidy not found")
endif()

find_program(CLANG_FORMAT_EXEC clang-format)
if(CLANG_FORMAT_EXEC)
    message(STATUS "Found clang-format")
    add_custom_target(
        format
        ${CLANG_FORMAT_EXEC} -i ${PROJECT_SOURCES} ${PROJECT_HEADERS}
        )
else()
    message(WARNING "clang-format not found")
endif()


