cmake_minimum_required(VERSION 3.16)

project(lambila VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    src/Design.h
    src/Logger.cpp
    src/Logger.h
    src/MainWindow.cpp
    src/MainWindow.h
    src/Project.cpp
    src/Project.h
    src/VhdlParser.cpp
    src/VhdlParser.h
    src/main.cpp
    ui/MainWindow.ui
    resources/resources.qrc
)

add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/src/_GitCommitHash.h"
    COMMAND git log --pretty=format:\"static const char *_GIT_COMMIT_HASH = \\\"%H\\\"$<SEMICOLON>\" -n 1 > "${CMAKE_SOURCE_DIR}/src/_GitCommitHash.h"
    DEPENDS ${PROJECT_SOURCES} CMakeLists.txt
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

qt_add_executable(lambila
    MANUAL_FINALIZATION
    src/_GitCommitHash.h
    ${PROJECT_SOURCES}
)

target_link_libraries(lambila PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

qt_finalize_executable(lambila)
