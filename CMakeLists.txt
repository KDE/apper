project(Apper)
cmake_minimum_required(VERSION 3.0)

set(APPER_VERSION 1.0.0)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(FindPkgConfig)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
include(FeatureSummary)
include(ECMInstallIcons)

find_package(Qt5 5.2.0 CONFIG REQUIRED Core DBus Widgets Quick Sql XmlPatterns)

# Load the frameworks we need
find_package(KF5 REQUIRED COMPONENTS
  Config
  DocTools
  GuiAddons
  I18n
  KCMUtils
  DBusAddons
  KIO
  Notifications
  IconThemes
)
find_package(LibKWorkspace REQUIRED)
find_package(KDED REQUIRED)
find_package(PackageKitQt5 1.0.0 REQUIRED)

#
# Options
#

# The various parts of Apper that can be built, or not.
option(BUILD_APPER "Build the Apper application" ON)
option(BUILD_APPERKCM "Build the Apper KDE Control Module" ON)
option(BUILD_APPERD "Build the Apper daemon" ON)
option(BUILD_DECLARATIVE "Build the Qt Quick plugins" ON)
option(BUILD_PKSESSION "Build the PkSession helper application" ON)
option(BUILD_PLASMOID "Build the update notifier plasmoid" ON)

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

# Enable support for Limba packages
option(LIMBA "Build Apper with Limba bundle support" OFF)
set(HAVE_LIMBA ${LIMBA})
message(STATUS "Building Apper with Limba support: " ${LIMBA})

option(MAINTAINER "Enable maintainer mode" OFF)

if(DEBCONF_SUPPORT)
    # Tries to find the package, when it finds it, set HAVE_DEBCONFKDE
    find_package(DebconfKDE REQUIRED)
    message(STATUS "Building with Debconf support")
    set(HAVE_DEBCONF ${DEBCONF_SUPPORT})
endif(DEBCONF_SUPPORT)

# command to edit the packages origins
set(EDIT_ORIGNS_DESKTOP_NAME "" CACHE STRING "Edit origins desktop name")
if (EDIT_ORIGNS_DESKTOP_NAME)
    message(STATUS "Edit origins desktop name: " ${EDIT_ORIGNS_DESKTOP_NAME})
endif(EDIT_ORIGNS_DESKTOP_NAME)

# Generate config.h
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)

#
# Add definitions
#
set(MAINTAINER_CFLAGS "")
if(MAINTAINER)
    set(MAINTAINER_CFLAGS "-Werror -Wall -Wcast-align -Wno-uninitialized -Wempty-body -Wformat-security -Winit-self -Wno-deprecated-declarations")
    if (CMAKE_COMPILER_IS_GNUCC)
        execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
                OUTPUT_VARIABLE GCC_VERSION)
        if (GCC_VERSION VERSION_GREATER 4.9 OR GCC_VERSION VERSION_EQUAL 4.9)
            set(MAINTAINER_CFLAGS ${MAINTAINER_CFLAGS} "-fdiagnostics-color=auto")
        endif()
    endif()
endif(MAINTAINER)
add_definitions(${MAINTAINER_CFLAGS})

add_definitions("-std=gnu++11")
add_definitions(
  -DQT_USE_QSTRINGBUILDER
  -DQT_STRICT_ITERATORS
  -DQT_NO_URL_CAST_FROM_STRING
  -DQT_NO_CAST_FROM_BYTEARRAY
  -DQT_NO_SIGNALS_SLOTS_KEYWORDS
  -DQT_USE_FAST_OPERATOR_PLUS
)

#
# Global includes
#
include_directories(${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PackageKitQt5_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/libapper)

#
# Subcomponents
#
add_subdirectory(libapper)
if(BUILD_APPERKCM)
    add_subdirectory(ApperKCM)
endif(BUILD_APPERKCM)
if(BUILD_APPER)
    add_subdirectory(Apper)
endif(BUILD_APPER)
if(BUILD_PKSESSION)
    add_subdirectory(PkSession)
endif(BUILD_PKSESSION)
if(BUILD_APPERD)
    add_subdirectory(apperd)
endif(BUILD_APPERD)
if(BUILD_DECLARATIVE)
    add_subdirectory(declarative-plugins)
endif(BUILD_DECLARATIVE)
if(BUILD_PLASMOID)
    add_subdirectory(plasmoid)
endif(BUILD_PLASMOID)
if(LIMBA)
    add_subdirectory(AppSetup)
endif()
add_subdirectory(doc)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
