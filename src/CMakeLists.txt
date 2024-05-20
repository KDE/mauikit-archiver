# SPDX-FileCopyrightText: 2020 Camilo Higuita <milo.h@aol.com>
#
# SPDX-License-Identifier: BSD-2-Clause

remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_URL_CAST_FROM_STRING)

set(LIB_NAME MauiKitArchiver${MAUI_MAJOR_VERSION})

ecm_add_qml_module(${LIB_NAME}
URI "org.mauikit.archiver"
CLASS_NAME ArchiverPlugin)

set(sources
    code/archiver_plugin.cpp
    code/compressedfile.cpp
    code/temporaryfile.cpp)

set(headers
    code/moduleinfo.h)

ecm_target_qml_sources(${LIB_NAME} SOURCES
    controls/ArchivePage.qml
    controls/NewArchiveDialog.qml)

target_sources(${LIB_NAME}
    PRIVATE
    ${sources})

add_library(MauiKit${MAUI_MAJOR_VERSION}::Archiver ALIAS MauiKitArchiver${MAUI_MAJOR_VERSION})

generate_export_header(${LIB_NAME} BASE_NAME Archiver)
set_target_properties(${LIB_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME "Archiver")

target_include_directories(${LIB_NAME}
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/MauiKit${MAUI_MAJOR_VERSION}/Archiver>")

target_include_directories(${LIB_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR};>")

if(ANDROID)
    target_link_libraries(${LIB_NAME} PRIVATE Qt${QT_MAJOR_VERSION}::AndroidExtras)
endif()

target_link_libraries(${LIB_NAME}
    PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Quick
    Qt${QT_MAJOR_VERSION}::Qml
    Qt${QT_MAJOR_VERSION}::Sql
    Qt${QT_MAJOR_VERSION}::Network
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::Archive
    KF${KF_MAJOR_VERSION}::CoreAddons
    MauiKit4
    MauiKit4::FileBrowsing)

ecm_finalize_qml_module(${LIB_NAME} DESTINATION ${KDE_INSTALL_QMLDIR})

install(TARGETS ${LIB_NAME} EXPORT ${LIB_NAME}Targets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${headers}
    ${CMAKE_CURRENT_BINARY_DIR}/archiver_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/MauiKit${MAUI_MAJOR_VERSION}/Archiver
    COMPONENT Devel)
