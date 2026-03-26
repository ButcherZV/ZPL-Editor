cmake_minimum_required(VERSION 3.20)
project(ZPLEditor VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── wxWidgets ─────────────────────────────────────────────────────────────────
# Build from MSYS2 UCRT64 terminal so wx-config is on PATH, e.g.:
#   cd /c/Users/Admin/Documents/GitHub/ZPL-Editor
#   cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
#   cmake --build build

set(wxWidgets_USE_UNICODE ON)

# Help CMake find wx-config when running inside MSYS2
if(NOT DEFINED wxWidgets_CONFIG_EXECUTABLE)
    find_program(wxWidgets_CONFIG_EXECUTABLE
        NAMES wx-config wx-config-3.2
        PATHS /ucrt64/bin /mingw64/bin /usr/bin
    )
endif()

find_package(wxWidgets REQUIRED
    COMPONENTS core base aui propgrid adv
)
include(${wxWidgets_USE_FILE})

# ── ZINT (barcode rendering) ──────────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
    zint
    GIT_REPOSITORY https://github.com/zint/zint.git
    GIT_TAG        2.13.0
    GIT_SHALLOW    TRUE
)
set(ZINT_USE_PNG OFF CACHE BOOL "" FORCE)
set(ZINT_USE_ZLIB OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(zint)

# ── Sources ───────────────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/AppConfig.cpp
    src/I18n.cpp
    src/MainFrame.cpp
    src/LabelCanvas.cpp
    src/ToolboxPanel.cpp
    src/PropertiesPanel.cpp
    src/ZPLCodePanel.cpp

    src/elements/LabelElement.cpp
    src/elements/TextElement.cpp
    src/elements/BarcodeElement.cpp
    src/elements/BoxElement.cpp
    src/elements/ImageElement.cpp

    src/commands/CommandHistory.cpp
    src/commands/AddElementCommand.cpp
    src/commands/MoveElementCommand.cpp
    src/commands/ResizeElementCommand.cpp
    src/commands/DeleteElementCommand.cpp
    src/commands/EditPropertyCommand.cpp

    src/zpl/ZPLSerializer.cpp
    src/zpl/ZPLParser.cpp

    src/dialogs/NewLabelDialog.cpp
    src/dialogs/AppOptionsDialog.cpp
)

# ── Executable ────────────────────────────────────────────────────────────────
if(WIN32)
    list(APPEND SOURCES app.rc)
    add_executable(ZPLEditor WIN32 ${SOURCES})
else()
    add_executable(ZPLEditor ${SOURCES})
endif()

target_include_directories(ZPLEditor PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${zint_SOURCE_DIR}/backend
)

target_link_libraries(ZPLEditor PRIVATE
    ${wxWidgets_LIBRARIES}
    zint
    $<$<PLATFORM_ID:Windows>:winspool>
)

# Windows: copy required DLLs next to the exe after build
if(WIN32)
    add_custom_command(TARGET ZPLEditor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:ZPLEditor>
            $<TARGET_FILE_DIR:ZPLEditor>
        COMMAND_EXPAND_LISTS
    )
endif()
