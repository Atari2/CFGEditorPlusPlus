cmake_minimum_required(VERSION 3.5)

project(CFGEditorPlusPlus LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(POSSIBLE_RELEASE_STRINGS "")
list(
    APPEND POSSIBLE_RELEASE_STRINGS
    "Release"
    "RelWithDebInfo"
    "MinSizeRel"
)

set(RELEASE_BUILD false)
foreach(POSSIBLE_STRING ${POSSIBLE_RELEASE_STRINGS})
    if (CMAKE_BUILD_TYPE STREQUAL ${POSSIBLE_STRING} OR CMAKE_CONFIGURATION_TYPES STREQUAL ${POSSIBLE_STRING})
        message(STATUS "Matched with ${POSSIBLE_STRING}")
        set(RELEASE_BUILD true)
        break()
    endif()
endforeach()

if (CFGEDITOR_BUILD_STATIC)
    if (NOT RELEASE_BUILD)
        message(FATAL_ERROR "CFGEDITOR_BUILD_STATIC was specified but the build is not of type Release")
    endif()
    if (NOT STATIC_QT_DIR)
        message(FATAL_ERROR "CFGEDITOR_BUILD_STATIC was specified but STATIC_QT_DIR was not passed")
    else()
        set(STATIC_BUILD true)
        set(CMAKE_PREFIX_PATH ${STATIC_QT_DIR})
    endif()
else()
    set(STATIC_BUILD false)
endif()

if (NOT DEFINED CMAKE_PREFIX_PATH)
    message(FATAL_ERROR "CMAKE_PREFIX_PATH is not set")
endif()

if (WIN32)
    set(ON_WINDOWS true)
    message(STATUS "Windows build")
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
        palettecontainer.cpp
        palettecontainer.h
        eightbyeightviewcontainer.cpp
        eightbyeightviewcontainer.h
        utils.h
        VioletEgg.rc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_resources(srcs_for_exe resources.qrc)
    qt_add_executable(CFGEditorPlusPlus
        ${PROJECT_SOURCES}
        ${srcs_for_exe}
    )
    if (WIN32 AND NOT STATIC_BUILD)
        get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
        get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
        find_program(WINDEPLOYQT_EXE windeployqt HINTS "${_qt_bin_dir}")
        if (RELEASE_BUILD)
            set(WINDEPLOY_CFG "release")
        else()
            set(WINDEPLOY_CFG "debug")
        endif()

        if (NOT STATIC_BUILD)
            message(STATUS "Building ${WINDEPLOY_CFG} windeploy")
            add_custom_command(TARGET CFGEditorPlusPlus POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E
                    env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXE}"
                        --${WINDEPLOY_CFG}
                        --no-compiler-runtime
                        --no-translations
                        "$<TARGET_FILE:CFGEditorPlusPlus>"
                COMMENT "Running windeployqt..."
            )
        endif()
    elseif(UNIX)
        install(TARGETS CFGEditorPlusPlus
            BUNDLE  DESTINATION .
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
        qt_generate_deploy_app_script(
            TARGET CFGEditorPlusPlus
            OUTPUT_SCRIPT deploy_script
            NO_UNSUPPORTED_PLATFORM_ERROR
        )
        install(SCRIPT ${deploy_script})
    endif()
else()
    add_executable(CFGEditorPlusPlus
            ${PROJECT_SOURCES}
    )
endif()

if (RELEASE_BUILD)
    message(STATUS "Building Release, removing qDebug()")
    target_compile_definitions(CFGEditorPlusPlus PRIVATE QT_NO_DEBUG_OUTPUT)
    if (ON_WINDOWS)
        target_link_options(CFGEditorPlusPlus PRIVATE /SUBSYSTEM:windows /ENTRY:mainCRTStartup)
    endif()
endif()
target_link_libraries(CFGEditorPlusPlus PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
