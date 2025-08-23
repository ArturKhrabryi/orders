cmake_minimum_required(VERSION 3.28 FATAL_ERROR)
project(orders LANGUAGES CXX VERSION 0.1.0)

option(BUILD_TO_ODS "Build python toOds converter with pyinstaller" ON)

if (BUILD_TO_ODS)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    set(TO_ODS_ENTRY "${CMAKE_SOURCE_DIR}/python/toOds.py")
    set(TO_ODS_REQS "${CMAKE_SOURCE_DIR}/python/requirements.txt")
    set(VENV_DIR "${CMAKE_BINARY_DIR}/pyenv")

    if (WIN32)
        set(PY_EXE "${VENV_DIR}/Scripts/python.exe")
        set(PIP_EXE "${VENV_DIR}/Scripts/pip.exe")
        set(TO_ODS_OUT "${CMAKE_BINARY_DIR}/pydist/toOds.exe")
    else()
        set(PY_EXE "${VENV_DIR}/bin/python")
        set(PIP_EXE "${VENV_DIR}/bin/pip")
        set(TO_ODS_OUT "${CMAKE_BINARY_DIR}/pydist/toOds")
    endif() 

    add_custom_command(
        OUTPUT "${TO_ODS_OUT}"
        BYPRODUCTS
            "${CMAKE_BINARY_DIR}/pyspec/toOds.spec"
        DEPENDS
            "${TO_ODS_ENTRY}"
            "${TO_ODS_REQS}"

        COMMAND "${CMAKE_COMMAND}" -E echo "Creating venv"
        COMMAND "${Python3_EXECUTABLE}" -m venv "${VENV_DIR}"

        COMMAND "${CMAKE_COMMAND}" -E echo "Upgrading tools"
        COMMAND "${PY_EXE}" -m pip install --upgrade pip setuptools wheel

        COMMAND "${CMAKE_COMMAND}" -E echo "Installing dependencies"
        COMMAND "${PIP_EXE}" install -r "${TO_ODS_REQS}"
        COMMAND "${PIP_EXE}" install pyinstaller

        COMMAND "${CMAKE_COMMAND}" -E echo "Packaging toOds with Pyinstaller"
        COMMAND "${PY_EXE}" -m PyInstaller
            --onefile
            --clean
            --noconfirm
            --name "toOds"
            --distpath "${CMAKE_BINARY_DIR}/pydist"
            --workpath "${CMAKE_BINARY_DIR}/pybuild"
            --specpath "${CMAKE_BINARY_DIR}/pyspec"
            "${TO_ODS_ENTRY}"
        VERBATIM
    )
    
    add_custom_target(${PROJECT_NAME}_toOds ALL DEPENDS "${TO_ODS_OUT}")

endif()

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
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_SCAN_FOR_MODULES OFF)
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Sql)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin COMPONENT main)
if (BUILD_TO_ODS)
    install(PROGRAMS "${TO_ODS_OUT}" DESTINATION bin COMPONENT toOds)
endif()

if (TARGET install AND BUILD_TO_ODS)
    add_dependencies(install ${PROJECT_NAME}_toOds)
endif()

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "Arturchik")
set(CPACK_PACKAGE_CONTACT "khrabryiartur2002@gmail.com")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "orders: Qt GUI app + toOds (PyInstaller)")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

set(CPACK_GENERATOR "TGZ;ZIP")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}")

set(CPACK_COMPONENTS_ALL main)
if (BUILD_TO_ODS)
    list(APPEND CPACK_COMPONENTS_ALL toOds)
endif()

set(CPACK_COMPONENT_MAIN_DISPLAY_NAME "C++ main GUI")
set(CPACK_COMPONENT_TOODS_DISPLAY_NAME "python to ods converter")

include(CPack)
