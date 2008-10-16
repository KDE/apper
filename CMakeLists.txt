cmake_minimum_required(VERSION 2.4.8)

project(KPackageKit)

find_package(KDE4 REQUIRED)
find_package(QPackageKit REQUIRED)
include(KDE4Defaults)

add_definitions(${QT_DEFINITIONS} ${KDE_DEFINITIONS})
include_directories( ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${QPACKAGEKIT_INCLUDE_DIR} )

#${KDE4_KIO_INCLUDES}
add_subdirectory( AddRm )
add_subdirectory( Settings )
add_subdirectory( Updater )
add_subdirectory( KPackageKit )
add_subdirectory( SmartIcon )
add_subdirectory( KPackageKitD )
add_subdirectory( Desktop )
