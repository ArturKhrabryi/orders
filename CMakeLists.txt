cmake_minimum_required(VERSION 3.28 FATAL_ERROR)
project(orders LANGUAGES CXX VERSION 0.1.0)

option(BUILD_TO_ODS "Build python to ods converter with pyinstaller" ON)
option(BUILD_BARCODE_GENERATOR "Build python barcode generator with pyinstaller" ON)

if (BUILD_TO_ODS OR BUILD_BARCODE_GENERATOR)
    include(cmake/PythonVenv.cmake)
    ensure_shared_venv(VENV_DIR PY_EXE PIP_EXE VENV_MARKER)
endif()

function(add_pyinstaller_tool TOOL_NAME ENTRY_FILE REQS_FILE OUT_BASENAME)
    set(dist "${CMAKE_BINARY_DIR}/pydist")
    set(work "${CMAKE_BINARY_DIR}/pybuild_${TOOL_NAME}")
    set(spec "${CMAKE_BINARY_DIR}/pyspec_${TOOL_NAME}")

    if (WIN32)
        set(out "${dist}/${OUT_BASENAME}.exe")
    else()
        set(out "${dist}/${OUT_BASENAME}")
    endif() 

    add_custom_command(
        OUTPUT "${out}"
        BYPRODUCTS "${spec}/${OUT_BASENAME}.spec"
        DEPENDS "${ENTRY_FILE}" "${REQS_FILE}" "${VENV_MARKER}"

        COMMAND "${CMAKE_COMMAND}" -E echo "Installing ${TOOL_NAME} deps"
        COMMAND "${PIP_EXE}" install -r "${REQS_FILE}"
        COMMAND "${PIP_EXE}" install pyinstaller

        COMMAND "${CMAKE_COMMAND}" -E echo "Packaging ${TOOL_NAME} with Pyinstaller"
        COMMAND "${PY_EXE}" -m PyInstaller
            --onefile
            --clean
            --noconfirm
            --name "${OUT_BASENAME}"
            --distpath "${dist}"
            --workpath "${work}"
            --specpath "${spec}"
            "${ENTRY_FILE}"
        VERBATIM USES_TERMINAL
    )

    add_custom_target(${PROJECT_NAME}_${TOOL_NAME} ALL DEPENDS "${out}")
    add_dependencies(${PROJECT_NAME}_${TOOL_NAME} py_venv)

    set(${TOOL_NAME}_OUT "${out}" PARENT_SCOPE)
endfunction()

set(ENABLED_COMPONENTS main)

if (BUILD_TO_ODS)
    add_pyinstaller_tool(
        toOds
        "${CMAKE_SOURCE_DIR}/python/toOds.py"
        "${CMAKE_SOURCE_DIR}/python/to_ods_requirements.txt"
        "toOds"
    )
    list(APPEND ENABLED_COMPONENTS toOds)
endif()

if (BUILD_BARCODE_GENERATOR)
    add_pyinstaller_tool(
        barcodeGenerator
        "${CMAKE_SOURCE_DIR}/python/barcodeGenerator.py"
        "${CMAKE_SOURCE_DIR}/python/barcode_generator_requirements.txt"
        "barcodeGenerator"
    )
    list(APPEND ENABLED_COMPONENTS barcodeGenerator)
endif()

find_package(Qt6 6 REQUIRED COMPONENTS Widgets Sql Svg LinguistTools)
qt_standard_project_setup(
    I18N_SOURCE_LANGUAGE en
    I18N_TRANSLATED_LANGUAGES pl ru
)

file (GLOB SOURCES CONFIGURE_DEPENDS src/*.cpp)
set(QT_OBJECT_INCLUDES
    include/MainWindow.hpp
    include/DatabaseProductsModel.hpp
    include/ProductFormWidget.hpp
    include/ProductsTableView.hpp
)
list (APPEND SOURCES ${QT_OBJECT_INCLUDES})

set(WIN32_FLAG)
if (WIN32)
    set(WIN32_FLAG WIN32)
endif()

qt_add_executable(${PROJECT_NAME} ${WIN32_FLAG} ${SOURCES})

file (GLOB ICONS CONFIGURE_DEPENDS res/icons/*.svg)
qt_add_resources(${PROJECT_NAME} app_icons
    PREFIX "/"
    BASE "res"
    FILES ${ICONS}
)

set(TS_FILES
    "${CMAKE_SOURCE_DIR}/translations/${PROJECT_NAME}_pl.ts"
    "${CMAKE_SOURCE_DIR}/translations/${PROJECT_NAME}_ru.ts"
)

file(GLOB HEADERS CONFIGURE_DEPENDS include/*.hpp)
qt_add_lupdate(${PROJECT_NAME}
    TS_FILES ${TS_FILES}
    SOURCES ${SOURCES} ${QT_OBJECT_INCLUDES} ${HEADERS}
    OPTIONS -no-obsolete
)

qt_add_lrelease(${PROJECT_NAME}
    TS_FILES ${TS_FILES}
    QM_FILES_OUTPUT_VARIABLE QM_FILES 
)

qt_add_resources(${PROJECT_NAME} i18n
    PREFIX "/i18n"
    BASE "${CMAKE_CURRENT_BINARY_DIR}"
    FILES ${QM_FILES}
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_SCAN_FOR_MODULES OFF)
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Sql Qt6::Svg)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin COMPONENT main)

if (BUILD_TO_ODS)
    install(PROGRAMS "${toOds_OUT}" DESTINATION bin COMPONENT toOds)
endif()

if (BUILD_BARCODE_GENERATOR)
    install(PROGRAMS "${barcodeGenerator_OUT}" DESTINATION bin COMPONENT barcodeGenerator)
endif()


set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "Arturchik")
set(CPACK_PACKAGE_CONTACT "khrabryiartur2002@gmail.com")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "orders: Qt GUI app + toOds + barcodeGenerator (PyInstaller)")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

set(CPACK_GENERATOR "TGZ;ZIP")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}")

set(CPACK_COMPONENTS_ALL ${ENABLED_COMPONENTS})

set(CPACK_COMPONENT_MAIN_DISPLAY_NAME "C++ main GUI")
set(CPACK_COMPONENT_TOODS_DISPLAY_NAME "python to ods converter")
set(CPACK_COMPONENT_BARCODEGENERATOR_DISPLAY_NAME "python barcode generator")

include(CPack)
