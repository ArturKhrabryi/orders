cmake_minimum_required(VERSION 3.28 FATAL_ERROR)
project(orders LANGUAGES CXX VERSION 0.1.0)

option(BUILD_TO_ODS "Build python to ods converter with pyinstaller" ON)
option(BUILD_BARCODE_GENERATOR "Build python barcode generator with pyinstaller" ON)

if (BUILD_TO_ODS OR BUILD_BARCODE_GENERATOR)
    include(cmake/PythonVenv.cmake)
    ensure_shared_venv(VENV_DIR PY_EXE PIP_EXE VENV_MARKER)
endif()

if (BUILD_TO_ODS)
    set(TO_ODS_ENTRY "${CMAKE_SOURCE_DIR}/python/toOds.py")
    set(TO_ODS_REQS "${CMAKE_SOURCE_DIR}/python/to_ods_requirements.txt")
    set(TO_ODS_DIST "${CMAKE_BINARY_DIR}/pydist")
    set(TO_ODS_WORK "${CMAKE_BINARY_DIR}/pybuild_toOds")
    set(TO_ODS_SPEC "${CMAKE_BINARY_DIR}/pyspec_toOds")

    if (WIN32)
        set(TO_ODS_OUT "${TO_ODS_DIST}/toOds.exe")
    else()
        set(TO_ODS_OUT "${TO_ODS_DIST}/toOds")
    endif() 

    add_custom_command(
        OUTPUT "${TO_ODS_OUT}"
        BYPRODUCTS "${TO_ODS_SPEC}/toOds.spec"
        DEPENDS
            "${TO_ODS_ENTRY}"
            "${TO_ODS_REQS}"
            "${VENV_MARKER}"

        COMMAND "${CMAKE_COMMAND}" -E echo "Installing toOds deps"
        COMMAND "${PIP_EXE}" install -r "${TO_ODS_REQS}"
        COMMAND "${PIP_EXE}" install pyinstaller

        COMMAND "${CMAKE_COMMAND}" -E echo "Packaging toOds with Pyinstaller"
        COMMAND "${PY_EXE}" -m PyInstaller
            --onefile
            --clean
            --noconfirm
            --name "toOds"
            --distpath "${TO_ODS_DIST}"
            --workpath "${TO_ODS_WORK}"
            --specpath "${TO_ODS_SPEC}"
            "${TO_ODS_ENTRY}"
            VERBATIM USES_TERMINAL
    )
    
    add_custom_target(${PROJECT_NAME}_toOds ALL DEPENDS "${TO_ODS_OUT}")
    add_dependencies(${PROJECT_NAME}_toOds py_venv)
endif()

if (BUILD_BARCODE_GENERATOR)
    set(BARCODE_GENERATOR_ENTRY "${CMAKE_SOURCE_DIR}/python/barcodeGenerator.py")
    set(BARCODE_GENERATOR_REQS "${CMAKE_SOURCE_DIR}/python/barcode_generator_requirements.txt")
    set(BARCODE_GENERATOR_DIST "${CMAKE_BINARY_DIR}/pydist")
    set(BARCODE_GENERATOR_WORK "${CMAKE_BINARY_DIR}/pybuild_barcodeGenerator")
    set(BARCODE_GENERATOR_SPEC "${CMAKE_BINARY_DIR}/pyspec_barcodeGenerator")

    if (WIN32)
        set(BARCODE_GENERATOR_OUT "${BARCODE_GENERATOR_DIST}/barcodeGenerator.exe")
    else()
        set(BARCODE_GENERATOR_OUT "${BARCODE_GENERATOR_DIST}/barcodeGenerator")
    endif() 

    add_custom_command(
        OUTPUT "${BARCODE_GENERATOR_OUT}"
        BYPRODUCTS "${BARCODE_GENERATOR_SPEC}/barcodeGenerator.spec"
        DEPENDS
            "${BARCODE_GENERATOR_ENTRY}"
            "${BARCODE_GENERATOR_REQS}"
            "${VENV_MARKER}"

        COMMAND "${CMAKE_COMMAND}" -E echo "Installing barcodeGenerator deps"
        COMMAND "${PIP_EXE}" install -r "${BARCODE_GENERATOR_REQS}"
        COMMAND "${PIP_EXE}" install pyinstaller

        COMMAND "${CMAKE_COMMAND}" -E echo "Packaging barcodeGenerator with Pyinstaller"
        COMMAND "${PY_EXE}" -m PyInstaller
            --onefile
            --clean
            --noconfirm
            --name "barcodeGenerator"
            --distpath "${BARCODE_GENERATOR_DIST}"
            --workpath "${BARCODE_GENERATOR_WORK}"
            --specpath "${BARCODE_GENERATOR_SPEC}"
            "${BARCODE_GENERATOR_ENTRY}"
            VERBATIM USES_TERMINAL
    )
    
    add_custom_target(${PROJECT_NAME}_barcodeGenerator ALL DEPENDS "${BARCODE_GENERATOR_OUT}")
    add_dependencies(${PROJECT_NAME}_barcodeGenerator py_venv)
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

if (BUILD_BARCODE_GENERATOR)
    install(PROGRAMS "${BARCODE_GENERATOR_OUT}" DESTINATION bin COMPONENT barcodeGenerator)
endif()

if (TARGET install)
    if (BUILD_TO_ODS)
        add_dependencies(install ${PROJECT_NAME}_toOds)
    endif()

    if (BUILD_BARCODE_GENERATOR)
        add_dependencies(install ${PROJECT_NAME}_barcodeGenerator)
    endif()
endif()


set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "Arturchik")
set(CPACK_PACKAGE_CONTACT "khrabryiartur2002@gmail.com")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "orders: Qt GUI app + toOds + barcodeGenerator (PyInstaller)")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

set(CPACK_GENERATOR "TGZ;ZIP")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}")

set(CPACK_COMPONENTS_ALL main)
if (BUILD_TO_ODS)
    list(APPEND CPACK_COMPONENTS_ALL toOds)
endif()

if (BUILD_BARCODE_GENERATOR)
    list(APPEND CPACK_COMPONENTS_ALL barcodeGenerator)
endif()

set(CPACK_COMPONENT_MAIN_DISPLAY_NAME "C++ main GUI")
set(CPACK_COMPONENT_TOODS_DISPLAY_NAME "python to ods converter")
set(CPACK_COMPONENT_BARCODEGENERATOR_DISPLAY_NAME "python barcode generator")

include(CPack)
