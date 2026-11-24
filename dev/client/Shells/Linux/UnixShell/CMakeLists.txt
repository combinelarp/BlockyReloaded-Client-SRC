# UnixShell CMakeLists (Linux)

message(STATUS "Configuring module: UnixShell...")

# Compute absolute paths from this directory
set(CLIENT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
set(ENGINE_CORE_DEV "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../engine-core/dev")
set(LOGIC_DEV "${CMAKE_CURRENT_SOURCE_DIR}/../../../../logic")

set(LORD_SRC_PATH "${ENGINE_CORE_DEV}/engine/Src")
set(LOGIC_SRC_PATH "${LOGIC_DEV}/Src")
set(LORD_DEP_PATH "${ENGINE_CORE_DEV}/engine/Dependencies")
set(LOGIC_DEP_PATH "${LOGIC_DEV}/dependencies")

# Bin paths (match the build type)
if(CMAKE_BUILD_TYPE MATCHES "Debug" OR CMAKE_BUILD_TYPE MATCHES "None")
    set(LOGIC_BIN_PATH "${LOGIC_DEV}/Bin/Debug")
    set(CORE_BIN_PATH "${ENGINE_CORE_DEV}/engine/Bin/Debug")
else()
    set(LOGIC_BIN_PATH "${LOGIC_DEV}/Bin/Release")
    set(CORE_BIN_PATH "${ENGINE_CORE_DEV}/engine/Bin/Release")
endif()

# Include directories
set(INCLUDE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CLIENT_ROOT}/Src
    ${CLIENT_ROOT}/Src/Blockman
    ${CLIENT_ROOT}/Src/External
    ${LORD_SRC_PATH}/Core
    ${LORD_SRC_PATH}/GLES2Render
    ${LORD_DEP_PATH}/RapidXml/Include
    ${LORD_DEP_PATH}/Lua/Linux/Include
    ${LORD_DEP_PATH}/Zlib/Linux/Include
    ${LORD_DEP_PATH}/rapidjson/Include
    ${LORD_DEP_PATH}/FreeType/Linux/Include
    ${LOGIC_SRC_PATH}
    ${LOGIC_DEP_PATH}/linux
    ${LOGIC_DEP_PATH}/cpr/linux/include
    ${LOGIC_DEP_PATH}/curl/linux/include
    ${LOGIC_DEP_PATH}/sparsehash/linux/include
    ${LORD_DEP_PATH}/g3log/linux/include
)

include_directories(${INCLUDE_FILES})

# Link directories
link_directories(
    ${LORD_DEP_PATH}/Lua/Linux/Lib
    ${LORD_DEP_PATH}/Zlib/Linux/Lib
    ${LORD_DEP_PATH}/FreeType/Linux/Lib
    ${LORD_DEP_PATH}/FreeImage/Linux/Lib
    ${LORD_DEP_PATH}/g3log/linux/lib
    ${LOGIC_DEP_PATH}/cpr/linux/lib
    ${LOGIC_DEP_PATH}/curl/linux/lib
    ${LOGIC_DEP_PATH}/linux/raknet/lib
    ${LOGIC_BIN_PATH}
    ${CORE_BIN_PATH}
)

# Source files
file(GLOB_RECURSE HEADER_FILES *.h)
file(GLOB_RECURSE SOURCE_FILES *.cpp)

# Exclude Windows-specific sources
list(FILTER SOURCE_FILES EXCLUDE REGEX ".*WinShell\\.cpp$")
list(FILTER SOURCE_FILES EXCLUDE REGEX ".*ShellInterfaceWin32\\.cpp$")
list(FILTER SOURCE_FILES EXCLUDE REGEX ".*Win32SoundEngine\\.cpp$")
list(FILTER SOURCE_FILES EXCLUDE REGEX ".*SimpleAudioEngine\\.cpp$")
list(FILTER SOURCE_FILES EXCLUDE REGEX ".*MciPlayer\\.cpp$")
list(FILTER SOURCE_FILES EXCLUDE REGEX ".*ConsoleUtility\\.cpp$")
list(FILTER SOURCE_FILES EXCLUDE REGEX ".*Test\\.cpp$")
list(FILTER SOURCE_FILES EXCLUDE REGEX ".*stdafx\\.cpp$")

add_definitions("-DLORD_GAMECLIENT_EXPORTS")
add_definitions("-DLORD_STATIC")
add_definitions("-DTOLUA_STATIC")

set(UNIXSHELL_NAME "UnixShell")
add_executable(${UNIXSHELL_NAME} ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(${UNIXSHELL_NAME}
    GameClient
    debug libLogic_d.a
    optimized libLogic.a
    debug libLordCore_d.a
    optimized libLordCore.a
    debug libLordGLES2Render_d.a
    optimized libLordGLES2Render.a
    lua
    g3logger
    FreeType
    FreeImage
    zlib
    cpr
    curl
    raknet
    pthread
    dl
    X11
    EGL
    GLESv2
)

set_target_properties(${UNIXSHELL_NAME} PROPERTIES DEBUG_OUTPUT_NAME "${UNIXSHELL_NAME}_d")
set_target_properties(${UNIXSHELL_NAME} PROPERTIES RELEASE_OUTPUT_NAME ${UNIXSHELL_NAME})

# Copy unixshell.ico next to the executable so the shell can load it at runtime
add_custom_command(TARGET ${UNIXSHELL_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/unixshell.ico"
        "$<TARGET_FILE_DIR:${UNIXSHELL_NAME}>/unixshell.ico"
    COMMENT "Copying unixshell.ico")

message(STATUS "Configure success!")