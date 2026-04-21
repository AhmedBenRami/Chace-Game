cmake_minimum_required(VERSION 3.10)
project(GameProject)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Place the executable in the project root (not in build/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# ── Local Raylib ──────────────────────────────────────────────────────────────
set(RAYLIB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/Raylib/include)

if(WIN32)
    set(RAYLIB_LIB ${CMAKE_SOURCE_DIR}/Raylib/windows/libraylib.a)
elseif(APPLE)
    set(RAYLIB_LIB ${CMAKE_SOURCE_DIR}/Raylib/macos/libraylib.a)
else()
    set(RAYLIB_LIB ${CMAKE_SOURCE_DIR}/Raylib/linux/libraylib.a)
endif()

# ── Sources ───────────────────────────────────────────────────────────────────
file(GLOB_RECURSE SRC_FILES
    "src/*.cpp"
    "main.cpp"
)

# ── Executable ────────────────────────────────────────────────────────────────
add_executable(${PROJECT_NAME} ${SRC_FILES})

# ── Include dirs ──────────────────────────────────────────────────────────────
target_include_directories(${PROJECT_NAME} PRIVATE
    ${RAYLIB_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/entities
    ${CMAKE_SOURCE_DIR}/src/game
    ${CMAKE_SOURCE_DIR}/src/map
    ${CMAKE_SOURCE_DIR}/src/menu
)

# ── Link ──────────────────────────────────────────────────────────────────────
target_link_libraries(${PROJECT_NAME} ${RAYLIB_LIB})

if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        winmm
        gdi32
        opengl32
    )
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME}
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework CoreFoundation"
    )
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME}
        GL
        m
        pthread
        dl
        rt
        X11
    )
endif()