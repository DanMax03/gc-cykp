cmake_minimum_required(VERSION 3.21)

project(gc-cykp)

set(CMAKE_CXX_STANDARD 17)

option(BUILD_TESTS "Ask cmake to build unit tests" OFF)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
        main.cpp
        src/Application.cpp
        src/ArgumentParsing.cpp
    PUBLIC
        src/ExceptionController.cpp
        src/Talker.cpp
        src/Grammar.cpp
        src/isInChomskyForm.cpp
        src/NonterminalCompression.cpp
        src/ChomskyFormConversion.cpp
        src/CYK_Algorithm.cpp
        src/execConversion.cpp
        src/execRecognition.cpp)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        "${PROJECT_SOURCE_DIR}/include")

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(BUILD_FLAGS "-g -DEXCEPTION_POLICY_INDEX=0")
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(BUILD_FLAGS "-DEXCEPTION_POLICY_INDEX=1")
endif()

target_compile_options(${PROJECT_NAME}
        PUBLIC "-g")  # TODO: fix BUILD_FLAGS

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")


if (${BUILD_TESTS})
    message("BUILD_TESTS is ON, so building tests...")
    add_subdirectory("${PROJECT_SOURCE_DIR}/testing")
endif()

