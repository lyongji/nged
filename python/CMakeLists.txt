# Python bindings (optional)
if(NOT PYTHON_EXECUTABLE STREQUAL "no")
    find_package(Python COMPONENTS Interpreter Development REQUIRED)

    # Generate parmexpr.lua.h
    set(PARMEXPR_LUA "${CMAKE_SOURCE_DIR}/deps/parmscript/parmexpr.lua")
    set(PARMEXPR_HEADER "${CMAKE_CURRENT_BINARY_DIR}/parmexpr.lua.h")

    add_custom_command(
        OUTPUT ${PARMEXPR_HEADER}
        COMMAND ${Python_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/bin2c.py" "${PARMEXPR_LUA}" "${PARMEXPR_HEADER}"
        DEPENDS "${PARMEXPR_LUA}" "${CMAKE_CURRENT_SOURCE_DIR}/bin2c.py"
        COMMENT "Generating parmexpr.lua.h"
    )

    # parmscript library
    add_library(parmscript STATIC
        ${PARMEXPR_HEADER}
        ${CMAKE_SOURCE_DIR}/deps/parmscript/inspectorext.cpp
        ${CMAKE_SOURCE_DIR}/deps/parmscript/jsonparm.cpp
        ${CMAKE_SOURCE_DIR}/deps/parmscript/parminspector.cpp
        ${CMAKE_SOURCE_DIR}/deps/parmscript/parmscript.cpp
        ${CMAKE_SOURCE_DIR}/deps/parmscript/pyparm.cpp
    )

    target_include_directories(parmscript PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_SOURCE_DIR}/deps/lua
        ${CMAKE_SOURCE_DIR}/deps/sol2/include
        ${CMAKE_SOURCE_DIR}/deps/nlohmann
        ${CMAKE_SOURCE_DIR}/deps/pybind11/include
        ${Python_INCLUDE_DIRS}
    )

    target_link_libraries(parmscript PUBLIC lua imgui nfd sol2::sol2 pybind11::headers)

    # ngpy Python extension
    pybind11_add_module(ngpy
        ${CMAKE_SOURCE_DIR}/src/ngpy.cpp
        ${CMAKE_SOURCE_DIR}/src/pybind11_imgui.cpp
    )

    # Output as a single file: ngpy.suffix.so
    set_target_properties(ngpy PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    set_target_properties(ngpy PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

    target_include_directories(ngpy PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/deps/boxer/include
        ${CMAKE_SOURCE_DIR}/deps/imgui
        ${CMAKE_SOURCE_DIR}/deps/nlohmann
        ${CMAKE_SOURCE_DIR}/deps/spdlog/include
        ${CMAKE_SOURCE_DIR}/deps/parallel_hashmap/parallel_hashmap
        ${CMAKE_SOURCE_DIR}/deps/subprocess.h
        ${CMAKE_SOURCE_DIR}/deps/parmscript
    )

    target_link_libraries(ngpy PRIVATE
        nged
        entry
        parmscript
    )

    if(MSVC)
        target_compile_options(ngpy PRIVATE /bigobj)
    endif()

    # Install the Python extension module into the wheel install-dir (nged/)
    # Note: wheel.install-dir in pyproject.toml is already set to "nged",
    # so DESTINATION here is relative to that — use "." to avoid double nesting.
    install(TARGETS ngpy
        LIBRARY DESTINATION . COMPONENT nged_python
        RUNTIME DESTINATION . COMPONENT nged_python
    )

    # Generate stubs after build
    # We need to run stubgen on the built module.
    # Since ngpy is now a single module (not a package), we point PYTHONPATH to the build dir.
    # Note: We use a custom command that runs *after* the build.
    add_custom_command(TARGET ngpy POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}" "${Python_EXECUTABLE}" -m pybind11_stubgen ngpy --output-dir "${CMAKE_CURRENT_BINARY_DIR}" --root-suffix "" --ignore-all-errors
        COMMENT "Generating Python stubs for ngpy"
        VERBATIM
    )

    # Install the generated stubs
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ngpy.pyi"
        DESTINATION .
        COMPONENT nged_python
        OPTIONAL # In case generation fails (though it shouldn't)
    )
endif()
