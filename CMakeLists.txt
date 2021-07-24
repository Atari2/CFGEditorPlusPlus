cmake_minimum_required(VERSION 3.5)

project(CFGEditorPlusPlus LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    set(ON_WINDOWS "True")
    if (CMAKE_CURRENT_SOURCE_DIR MATCHES ".*[\//]aless[\//].*")
        message(STATUS "In aless PC")
        set(CMAKE_PREFIX_PATH "F:/Qt/6.0.3/msvc2019_64")
    else()
        message(STATUS "Extraneous Build")
        set(CMAKE_PREFIX_PATH "C:/Qt/6.0.4/msvc2019_64")
    endif()
else()
    message(STATUS "Unix build")
endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        cfgeditor.cpp
        cfgeditor.h
        cfgeditor.ui
        jsonsprite.cpp
        jsonsprite.h
        tweak_bytes.cpp
        tweak_bytes.h
        spritepalettecreator.cpp
        spritepalettecreator.h
        spritedatamodel.cpp
        spritedatamodel.h
        snesgfxconverter.cpp
        snesgfxconverter.h
        eightbyeightview.cpp
        eightbyeightview.h
        paletteview.cpp
        paletteview.h
        map16provider.cpp
        map16provider.h
        map16graphicsview.cpp
        map16graphicsview.h
        clipboardtile.cpp
        clipboardtile.h
        utils.h
        VioletEgg.rc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_resources(srcs_for_exe resources.qrc)
    qt_add_executable(CFGEditorPlusPlus
        ${PROJECT_SOURCES}
        ${srcs_for_exe}
    )
    get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    find_program(WINDEPLOYQT_EXE windeployqt HINTS "${_qt_bin_dir}")
if (WIN32)
    if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_CONFIGURATION_TYPES STREQUAL "Release")
        message(STATUS "Building release windeploy")
        add_custom_command(TARGET CFGEditorPlusPlus POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E
                env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXE}"
                    --release
                    --no-compiler-runtime
                    --no-translations
                    "$<TARGET_FILE:CFGEditorPlusPlus>"
            COMMENT "Running windeployqt..."
        )
    endif()
endif()
else()
    add_executable(CFGEditorPlusPlus
            ${PROJECT_SOURCES}
    )
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_CONFIGURATION_TYPES STREQUAL "Release")
    message(STATUS "Building Release, removing qDebug()")
    target_compile_definitions(CFGEditorPlusPlus PRIVATE QT_NO_DEBUG_OUTPUT)
    target_link_options(CFGEditorPlusPlus PRIVATE /SUBSYSTEM:windows /ENTRY:mainCRTStartup)
endif()

target_link_libraries(CFGEditorPlusPlus PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
