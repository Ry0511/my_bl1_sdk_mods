cmake_minimum_required(VERSION 3.23)
project(bl1_sdk_mods)

find_package(Python3 REQUIRED)

message(STATUS "CMakeBuildType -> ${CMAKE_BUILD_TYPE}, ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

if (${BL1_MODS_PRESET} EQUAL "")
    message(FATAL_ERROR "Invalid configuration preset")
endif ()

add_custom_target(${PROJECT_NAME})
add_custom_target(bl1_package_all)

add_custom_target(
        bl1_clean_install_dir
        COMMAND
        ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_SOURCE_DIR}/build/${BL1_MODS_PRESET}"
)

add_subdirectory(./vendor/pyunrealsdk EXCLUDE_FROM_ALL)
add_dependencies(${PROJECT_NAME} pyunrealsdk)

################################################################################
# | UTILITY FUNCTIONS |
################################################################################

function(bl1_create_packager target dir)
    # Don't even try to package non-release stuff
    if (NOT ${CMAKE_BUILD_TYPE} MATCHES "Release")
        return()
    endif ()

    # Packages the contents of the directory as is producing; 'target-ver.zip'
    add_custom_target(
            _packager_${target}
            COMMAND
            "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/scripts/package_release_zip.py"
            "--dir-to-zip \"${dir}\""
            "--out \"${CMAKE_CURRENT_SOURCE_DIR}/packaged/\""
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/scripts"
    )
    add_dependencies(bl1_package_all _packager_${target})

endfunction()

function(bl1_add_py_module target dir)

    # Grab All Files
    file(GLOB_RECURSE DIR_CONTENT "${dir}/*.*")
    list(FILTER DIR_CONTENT EXCLUDE REGEX ".+\\.(cpp)|(h)")
    message(STATUS "Directory Content: ${DIR_CONTENT}")

    # Grab All C++ Sources
    file(GLOB_RECURSE "CPP_SOURCES" "${dir}/*.cpp" "${dir}/*.h")

    # Add the module
    pyunrealsdk_add_module(${target} "${CPP_SOURCES}")
    add_dependencies(${PROJECT_NAME} ${target})

    # Install
    string(REGEX REPLACE "/$" "" dir ${dir})
    get_filename_component(BL1_DIR_NAME ${dir} NAME)
    set(INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/${BL1_MODS_PRESET}/${BL1_DIR_NAME}")
    install(TARGETS ${target} DESTINATION ${INSTALL_DIR})
    install(DIRECTORY "${dir}/"  DESTINATION  ${INSTALL_DIR})

    # Packaging
    bl1_create_packager(${target} ${INSTALL_DIR})

    message(STATUS "Native Module Created; Install Dir: '${INSTALL_DIR}'")

endfunction()

function(bl1_add_basic_py_module name dir)
    set(INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/${BL1_MODS_PRESET}/py/${name}")
    install(DIRECTORY "${dir}" DESTINATION ${INSTALL_DIR})
    bl1_create_packager(${name} ${INSTALL_DIR})
endfunction()

################################################################################
# | ADDING MODULES |
################################################################################

# Leaked project
add_subdirectory(./src/native/object_explorer)
bl1_add_py_module(object_explorer "src/native/object_explorer")
target_link_libraries(object_explorer PRIVATE imgui_glfw_gl3)
target_compile_definitions(object_explorer PRIVATE _CRT_SECURE_NO_WARNINGS)

bl1_add_py_module(package_tools "src/native/package_tools")
#bl1_add_py_module(blutils "src/native/blutils")

# You need to install these yourself if building
#if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
#    find_package(antlr4-runtime REQUIRED PATHS "$ENV{ANTLR_RUNTIME_INSTALL_DIR}/release")
#else ()
#    find_package(antlr4-runtime REQUIRED PATHS "$ENV{ANTLR_RUNTIME_INSTALL_DIR}/debug")
#endif ()

#include_directories(${ANTLR4_INCLUDE_DIR})
#add_dependencies(blutils antlr4_static)
#target_link_libraries(blutils PRIVATE antlr4_static)

bl1_add_basic_py_module("rys_qol" "src/py/rys_qol/")
bl1_add_basic_py_module("rys_auto_save" "src/py/rys_auto_save/")
bl1_add_basic_py_module("skill_tree_tweaks" "src/py/skill_tree_tweaks/")