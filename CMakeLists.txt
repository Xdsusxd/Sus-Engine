cmake_minimum_required(VERSION 3.28)
project(FYEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── FetchContent dependencies ──────────────────────────────────
include(FetchContent)

# GLFW 3.4
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(glm)

# Vulkan SDK
find_package(Vulkan REQUIRED)
message(STATUS "Vulkan found: ${Vulkan_LIBRARY}")

# VMA (Vulkan Memory Allocator) — header-only
FetchContent_Declare(
    vma
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG        v3.2.1
)
FetchContent_MakeAvailable(vma)

# ── Engine library ─────────────────────────────────────────────
file(GLOB_RECURSE ENGINE_SOURCES
    "engine/core/*.cpp"
    "engine/renderer/*.cpp"
    "engine/scene/*.cpp"
    "engine/physics/*.cpp"
    "engine/animation/*.cpp"
    "engine/audio/*.cpp"
    "engine/ai/*.cpp"
    "engine/ui/*.cpp"
    "engine/tools/*.cpp"
)

add_library(FYEngineLib STATIC ${ENGINE_SOURCES})

target_include_directories(FYEngineLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/engine
)

target_link_libraries(FYEngineLib PUBLIC
    glfw
    glm::glm
    Vulkan::Vulkan
)

# MSVC specific flags
if(MSVC)
    target_compile_options(FYEngineLib PRIVATE /W4 /EHsc /utf-8)
    target_compile_definitions(FYEngineLib PRIVATE
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
endif()

# ── Game executable ────────────────────────────────────────────
file(GLOB_RECURSE GAME_SOURCES "game/*.cpp")

add_executable(FYEngine ${GAME_SOURCES})

target_link_libraries(FYEngine PRIVATE FYEngineLib)

if(MSVC)
    target_compile_options(FYEngine PRIVATE /W4 /EHsc /utf-8)
endif()

# Copy assets to build directory
add_custom_command(TARGET FYEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:FYEngine>/assets"
    COMMENT "Copying assets..."
)
