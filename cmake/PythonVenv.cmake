function(ensure_shared_venv VENV_DIR_OUT PY_EXE_OUT PIP_EXE_OUT MARKER_OUT)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    set(VENV_DIR "${CMAKE_BINARY_DIR}/pyenv")
    if (WIN32)
        set(PY_EXE "${VENV_DIR}/Scripts/python.exe")
        set(PIP_EXE "${VENV_DIR}/Scripts/pip.exe")
    else()
        set(PY_EXE "${VENV_DIR}/bin/python")
        set(PIP_EXE "${VENV_DIR}/bin/pip")
    endif() 
    set(VENV_MARKER "${VENV_DIR}/.venv_ready")

    if (NOT TARGET py_venv)
        add_custom_command(
            OUTPUT "${VENV_MARKER}"
            COMMAND "${Python3_EXECUTABLE}" -m venv "${VENV_DIR}"
            COMMAND "${PY_EXE}" -m pip install --upgrade pip setuptools wheel
            COMMAND "${CMAKE_COMMAND}" -E touch "${VENV_MARKER}"
            VERBATIM
        )
        add_custom_target(py_venv DEPENDS "${VENV_MARKER}")
    endif()

    set(${VENV_DIR_OUT} "${VENV_DIR}" PARENT_SCOPE)
    set(${PY_EXE_OUT} "${PY_EXE}" PARENT_SCOPE)
    set(${PIP_EXE_OUT} "${PIP_EXE}" PARENT_SCOPE)
    set(${MARKER_OUT} "${VENV_MARKER}" PARENT_SCOPE)
endfunction()
