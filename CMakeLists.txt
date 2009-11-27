project(KPackageKit)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/)

find_package(KDE4 REQUIRED)

include(FindPkgConfig)
pkg_check_modules (QPACKAGEKIT REQUIRED packagekit-qt>=0.5.5)
include(KDE4Defaults)

add_definitions(${QT_DEFINITIONS} ${KDE_DEFINITIONS})
include_directories( ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} 
                     ${KDE4_INCLUDES} ${QPACKAGEKIT_INCLUDE_DIRS} ${QPACKAGEKIT_INCLUDEDIR}/PackageKit/packagekit-qt
                     ${CMAKE_CURRENT_SOURCE_DIR}/libkpackagekit )

add_subdirectory( libkpackagekit)
add_subdirectory( AddRm )
add_subdirectory( Settings )
add_subdirectory( Updater )
add_subdirectory( KPackageKit )
add_subdirectory( SmartIcon )
add_subdirectory( KPackageKitD )
add_subdirectory( Desktop )
