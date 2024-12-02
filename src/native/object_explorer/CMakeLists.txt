cmake_minimum_required(VERSION 3.23)
project(object_explorer)

find_package(OpenGL REQUIRED)

set(GLFW_BUILD_SHARED_LIBS OFF CACHE BOOL "Build static GLFW library" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Don't build GLFW examples" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Don't build GLFW tests" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Don't build GLFW docs" FORCE)

fetchcontent_declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
)
fetchcontent_makeavailable(glfw)

# Fetch ImGui with Docking support
fetchcontent_declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG docking
)
fetchcontent_makeavailable(imgui)

# Imgui is not a cmake project so we will have to setup manually
add_library(imgui_glfw_gl3 STATIC)
target_include_directories(imgui_glfw_gl3 PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)

file(GLOB IMGUI_BASIC_SRC "${imgui_SOURCE_DIR}/*.cpp" "${imgui_SOURCE_DIR}/*.h")
target_sources(imgui_glfw_gl3 PUBLIC ${IMGUI_BASIC_SRC})
target_sources(
        imgui_glfw_gl3
        PUBLIC
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.h
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.h
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.h
        ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

target_link_libraries(imgui_glfw_gl3 PUBLIC glfw OpenGL::GL)

# add_library(object_explorer SHARED object_explorer.cpp)
# target_link_libraries(object_explorer PRIVATE unrealsdk imgui_glfw_gl3)
