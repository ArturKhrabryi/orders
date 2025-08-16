cmake_minimum_required(VERSION 3.21 FATAL_ERROR)
project(orders LANGUAGES CXX)
find_package(Qt6 6 REQUIRED COMPONENTS Widgets Sql)

qt_standard_project_setup()

file (GLOB SOURCES CONFIGURE_DEPENDS src/*.cpp)
list (APPEND SOURCES include/MainWindow.hpp)

if (WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${SOURCES})
else()
    add_executable(${PROJECT_NAME} ${SOURCES})
endif()

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Sql)
