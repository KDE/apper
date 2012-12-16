project(Apper)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/)

find_package(KDE4 REQUIRED)
find_package(KDE4Internal REQUIRED)
find_package(KDE4Workspace REQUIRED)
include (MacroLibrary)

# Only for Debian based systems
option(DEBCONF_SUPPORT "Build Apper with debconf support" OFF)

# Yum does not support this
option(AUTOREMOVE "Build Apper with auto remove enabled" OFF)
set(HAVE_AUTOREMOVE ${AUTOREMOVE})
message(STATUS "Building Apper with auto remove: " ${AUTOREMOVE})

# AppStream application management support
option(APPSTREAM "Build Apper with AppStream support" OFF)
set(HAVE_APPSTREAM ${APPSTREAM})
message(STATUS "Building Apper with AppStream support: " ${APPSTREAM})

# Enable support for Listaller cross-distro application installer
option(LISTALLER "Build Apper with app install" OFF)
set(HAVE_LISTALLER ${LISTALLER})
message(STATUS "Building Apper with Listaller modules: " ${LISTALLER})

if (HAVE_APPSTREAM)
    # path to the categories.xml menus
    set(AS_CATEGORIES_PATH "${CMAKE_INSTALL_PREFIX}/share/app-info"
        CACHE STRING "AppStream categories path")
    message(STATUS "AppStream categories path: " ${AS_CATEGORIES_PATH})
endif (HAVE_APPSTREAM)

if (DEBCONF_SUPPORT)
    # Tries to find the package, when it finds it HAVE_DEBCONFKDE is set in config.h
    find_package(DebconfKDE REQUIRED)
    macro_bool_to_01(DebconfKDE_FOUND HAVE_DEBCONFKDE)
    macro_log_feature(DebconfKDE_FOUND "DebconfKDE" "The KDE frontend for debconf" "http://websvn.kde.org" FALSE "" "Library used for showing debconf questions.")
    message(STATUS "Building with Debconf support")
endif (DEBCONF_SUPPORT)


# command to edit the packages origins
set(EDIT_ORIGNS_DESKTOP_NAME "" CACHE STRING "Edit origins desktop name")
if (EDIT_ORIGNS_DESKTOP_NAME)
    message(STATUS "Edit origins desktop name: " ${EDIT_ORIGNS_DESKTOP_NAME})
endif (EDIT_ORIGNS_DESKTOP_NAME)

# Generate config.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)

include(FindPkgConfig)
set(QPACKAGEKIT_MIN_VERSION 0.8.5)
pkg_check_modules(QPACKAGEKIT2 REQUIRED packagekit-qt2>=${QPACKAGEKIT_MIN_VERSION})
macro_log_feature(QPACKAGEKIT2_FOUND "packagekit-qt2" "The Qt library to use PackageKit" "http://www.packagekit.org" TRUE ${QPACKAGEKIT_MIN_VERSION} "")
include(KDE4Defaults)

add_definitions(${QT_DEFINITIONS}
    ${KDE_DEFINITIONS})

include_directories(${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE4_INCLUDES}
    ${QPACKAGEKIT2_INCLUDE_DIRS}
    ${QPACKAGEKIT2_INCLUDEDIR}/PackageKit/packagekit-qt2
    ${CMAKE_CURRENT_SOURCE_DIR}/libapper)

message(STATUS "QPK includ dir: " ${QPACKAGEKIT2_INCLUDEDIR}/PackageKit/packagekit-qt2)

add_subdirectory(libapper)
add_subdirectory(ApperKCM)
add_subdirectory(Apper)
add_subdirectory(PkSession)
add_subdirectory(apperd)
add_subdirectory(Desktop)
add_subdirectory(plasmoid)
add_subdirectory(doc)
if(LISTALLER)
    add_subdirectory(AppSetup)
endif()
