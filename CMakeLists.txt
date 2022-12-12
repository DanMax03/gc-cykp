cmake_minimum_required(VERSION 3.21)

project(gc-cykp)

set(CMAKE_CXX_STANDARD 17)

option(BUILD_TESTS "Ask cmake to build unit tests")


add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
        main.cpp
    PUBLIC
        src/Talker.cpp
        src/Grammar.cpp
        src/isInChomskyForm.cpp
        src/ChomskyFormConvertation.cpp
        src/CYK_Algorithm.cpp
        src/execConvertation.cpp
        src/execRecognition.cpp)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        "${PROJECT_SOURCE_DIR}/include")

target_compile_options(${PROJECT_NAME}
    PUBLIC -g)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")


if (${BUILD_TESTS})
    add_subdirectory("${PROJECT_SOURCE_DIR}/testing")
endif()

