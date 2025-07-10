cmake_minimum_required(VERSION 3.23)
project(treevis)

################################################################################
# | TREEVIS TARGET |
################################################################################

add_executable(treevis)
target_sources(treevis PRIVATE main.cpp)
target_compile_features(treevis PRIVATE cxx_std_20)

target_link_libraries(treevis PRIVATE tm_parse_solo)

################################################################################
# | SETUP | 100% ai generated
################################################################################

include(FetchContent)

# --- Fetch GLFW ---
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        latest
)
FetchContent_MakeAvailable(glfw)

# --- Fetch ImGui ---
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        docking
)
FetchContent_MakeAvailable(imgui)

# ImGui source files (manually add backends if needed)
target_sources(treevis PRIVATE
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
)

# Example: Add ImGui GLFW + OpenGL3 backend
target_sources(treevis PRIVATE
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# Include directories
target_include_directories(treevis PRIVATE
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
)

# Link GLFW and OpenGL (adjust depending on platform)
find_package(OpenGL REQUIRED)
target_link_libraries(treevis PRIVATE glfw OpenGL::GL)