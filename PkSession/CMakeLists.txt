# CMakeLists for Apper PackageKit Session Interface

set(pksession_SRCS
    AbstractIsRunning.cpp
    FilesModel.cpp
    IntroDialog.cpp
    ReviewChanges.cpp
    SessionTask.cpp
    PkInterface.cpp
    PkInstallPackageNames.cpp
    PkSearchFile.cpp
    PkIsInstalled.cpp
    PkInstallMimeTypes.cpp
    PkInstallGStreamerResources.cpp
    PkInstallFontconfigResources.cpp
    PkInstallPlasmaResources.cpp
    PkInstallPackageFiles.cpp
    PkInstallProvideFiles.cpp
    PkInstallCatalogs.cpp
    PkRemovePackageByFiles.cpp
    PkInstallPrinterDrivers.cpp
    PkSession.cpp
    main.cpp
)

qt5_add_dbus_adaptor(pksession_SRCS
    org.freedesktop.PackageKit.xml
    PkInterface.h
    PkInterface
)

qt5_wrap_ui(pksession_SRCS
    IntroDialog.ui
    ReviewChanges.ui
    SessionTask.ui
)

add_executable(apper-pk-session
    ${pksession_SRCS}
)

target_link_libraries(apper-pk-session
    KF5::WidgetsAddons
    KF5::KIOFileWidgets
    KF5::KDELibs4Support
    KF5::IconThemes
    ${PackageKitQt5_LIBRARIES}
    apper_private
)

qt5_use_modules(apper-pk-session Core XmlPatterns)

set_target_properties(apper-pk-session PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR}/apper)

install(TARGETS apper-pk-session DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})

include(MacroDBusAddActivationService)
dbus_add_activation_service(org.freedesktop.PackageKit.service.in)
