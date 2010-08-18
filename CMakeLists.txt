project(KPackageKit)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/)

find_package(KDE4 REQUIRED)
include (MacroLibrary)

# Only for Debian based systems
option(DEBCONF_SUPPORT "Build KPackageKit with debconf support" OFF)

# Yum does not support this
option(AUTOREMOVE "Build KPackageKit with auto remove enabled" ON)
set(HAVE_AUTOREMOVE ${AUTOREMOVE})
message(STATUS "Building KPackageKit with auto remove enabled: " ${AUTOREMOVE})

if (DEBCONF_SUPPORT)
    # Tries to find the package, when it finds it HAVE_DEBCONFKDE is set in config.h
    find_package(DebconfKDE REQUIRED)
    macro_bool_to_01(DebconfKDE_FOUND HAVE_DEBCONFKDE)
    macro_log_feature(DebconfKDE_FOUND "DebconfKDE" "The KDE frontend for debconf" "http://websvn.kde.org" FALSE "" "Library used for showing debconf questions.")
    message(STATUS "Building with Debconf support")
endif (DEBCONF_SUPPORT)

# Generate config.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)

include(FindPkgConfig)
pkg_check_modules(QPACKAGEKIT REQUIRED packagekit-qt>=0.6.3)
include(KDE4Defaults)

add_definitions(${QT_DEFINITIONS}
    ${KDE_DEFINITIONS})

include_directories(${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE4_INCLUDES}
    ${QPACKAGEKIT_INCLUDE_DIRS}
    ${QPACKAGEKIT_INCLUDEDIR}/PackageKit/packagekit-qt
    ${CMAKE_CURRENT_SOURCE_DIR}/libkpackagekit)

add_subdirectory(libkpackagekit)
add_subdirectory(AddRm)
add_subdirectory(Settings)
add_subdirectory(Updater)
add_subdirectory(KPackageKit)
add_subdirectory(SmartIcon)
add_subdirectory(KPackageKitD)
add_subdirectory(Desktop)
