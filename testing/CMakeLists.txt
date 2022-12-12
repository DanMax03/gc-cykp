set(PARENT_PROJECT_NAME "${PROJECT_NAME}")
project(gc-cykp-ut)

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG main
)

# For Windows: prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)


get_target_property(PARENT_SOURCES "${PARENT_PROJECT_NAME}" INTERFACE_SOURCES)
get_target_property(PARENT_INCLUDES "${PARENT_PROJECT_NAME}" INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(PARENT_RUNTIME_OUTPUT_DIR "${PARENT_PROJECT_NAME}" RUNTIME_OUTPUT_DIRECTORY)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
        main.cpp
        ${PARENT_SOURCES}
        Grammar.test.cpp)

target_link_libraries(${PROJECT_NAME}
    GTest::gtest_main)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${PARENT_INCLUDES})

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PARENT_RUNTIME_OUTPUT_DIR}")

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules")

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CODE_COVERAGE_VERBOSE ON)
    include(CodeCoverage)
    setup_target_for_coverage_lcov(
        NAME ${PARENT_PROJECT_NAME}_coverage
        EXECUTABLE ${PROJECT_NAME})
endif()
