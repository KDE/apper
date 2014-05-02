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

if (DEBCONF_SUPPORT)
    # Tries to find the package, when it finds it HAVE_DEBCONFKDE is set in config.h
    find_package(DebconfKDE REQUIRED)
    macro_bool_to_01(DebconfKDE_FOUND HAVE_DEBCONFKDE)
    macro_log_feature(DebconfKDE_FOUND "DebconfKDE" "The KDE frontend for debconf" "http://websvn.kde.org" FALSE "" "Library used for showing debconf questions.")
    message(STATUS "Building with Debconf support")
endif (DEBCONF_SUPPORT)

set(APP_VERSION 0.9.0)

# command to edit the packages origins
set(EDIT_ORIGNS_DESKTOP_NAME "" CACHE STRING "Edit origins desktop name")
if (EDIT_ORIGNS_DESKTOP_NAME)
    message(STATUS "Edit origins desktop name: " ${EDIT_ORIGNS_DESKTOP_NAME})
endif (EDIT_ORIGNS_DESKTOP_NAME)

# Generate config.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)

include(FindPkgConfig)

find_package(PackageKitQt4 0.9.0 REQUIRED)
include(KDE4Defaults)

add_definitions(${QT_DEFINITIONS}
    ${KDE_DEFINITIONS})

add_definitions("-std=gnu++11")

include_directories(${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE4_INCLUDES}
    ${PackageKitQt4_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/libapper)

message(STATUS "PackageKit include dir: " ${PackageKitQt4_INCLUDE_DIR})

add_subdirectory(libapper)
add_subdirectory(ApperKCM)
add_subdirectory(Apper)
add_subdirectory(PkSession)
add_subdirectory(apperd)
add_subdirectory(declarative-plugins)
add_subdirectory(plasmoid)
add_subdirectory(doc)
if(LISTALLER)
    add_subdirectory(AppSetup)
endif()
