cmake_minimum_required(VERSION 3.5)

project(CFGEditorPlusPlus LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check https://doc.qt.io/qt/deployment-android.html for more information.
# They need to be set before the find_package( ...) calls below.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

if (CMAKE_CURRENT_SOURCE_DIR MATCHES ".*[\//]aless[\//].*")
    message(STATUS "In aless PC")
    set(CMAKE_PREFIX_PATH "F:/Qt/6.0.3/msvc2019_64")
else()
    message(STATUS "CI Build")
    set(CMAKE_PREFIX_PATH "C:/Qt/6.0.1/msvc2019_64")
endif()
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools REQUIRED)

set(TS_FILES CFGEditorPlusPlus_en_US.ts)

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
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_resources(srcs_for_exe palettes.qrc)
    qt_add_executable(CFGEditorPlusPlus
        ${PROJECT_SOURCES}
        ${srcs_for_exe}
    )

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(CFGEditorPlusPlus SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(CFGEditorPlusPlus
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(CFGEditorPlusPlus PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
