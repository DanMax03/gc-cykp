cmake_minimum_required(VERSION 3.21)

project(untitled)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME}
        main.cpp
        src/Talker.cpp
        src/Grammar.cpp
        src/ChomskyFormConvertation.cpp
        src/execConvertation.cpp
        src/execRecognition.cpp)

target_include_directories(${PROJECT_NAME}
        PUBLIC include)

