find_package(KDE4Internal REQUIRED)
find_package(KDE4Workspace REQUIRED)

set(sentinel_SRCS
    StatusNotifierItem.cpp
    TransactionTrayIcon.cpp
    TransactionJob.cpp
    TransactionWatcher.cpp
    IntroDialog.cpp
    InfoWidget.cpp
    AbstractIsRunning.cpp
    UpdateIcon.cpp
    DistroUpgrade.cpp
    DBusInterface.cpp
    PkInterface.cpp
    SessionTask.cpp
    PkInstallPackageNames.cpp
    PkSearchFile.cpp
    PkIsInstalled.cpp
    PkInstallMimeTypes.cpp
    PkInstallGStreamerResources.cpp
    PkInstallFontconfigResources.cpp
    PkInstallPlasmaResources.cpp
    FilesModel.cpp
    PkInstallPackageFiles.cpp
    PkInstallProvideFiles.cpp
    PkInstallCatalogs.cpp
    PkRemovePackageByFiles.cpp
    PkInstallPrinterDrivers.cpp
    ReviewChanges.cpp
    ApperSentinel.cpp
    main.cpp
)

kde4_add_ui_files(sentinel_SRCS
    SessionTask.ui
    IntroDialog.ui
    InfoWidget.ui
    ReviewChanges.ui
)

qt4_add_dbus_adaptor(sentinel_dbus_SRCS
    org.kde.ApperSentinel.xml
    DBusInterface.h
    DBusInterface
)

qt4_add_dbus_adaptor(sentinel_dbus_SRCS
    org.freedesktop.PackageKit.xml
    PkInterface.h
    PkInterface
)

kde4_add_executable(apper-sentinel
    ${sentinel_SRCS}
    ${sentinel_dbus_SRCS}
)

target_link_libraries(apper-sentinel
    ${KDE4WORKSPACE_KWORKSPACE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_SOLID_LIBS}
    ${QPACKAGEKIT2_LIBRARIES}
    ${DEBCONF_KDE_LIB}
    ${QT_QTXMLPATTERNS_LIBRARY}
    apper
)

set_target_properties(apper-sentinel PROPERTIES INSTALL_RPATH ${LIB_INSTALL_DIR}/apper)

install(TARGETS apper-sentinel DESTINATION ${LIBEXEC_INSTALL_DIR})

install(FILES ApperSentinel.notifyrc DESTINATION ${DATA_INSTALL_DIR}/ApperSentinel)

include(PkgConfigGetVar)
dbus_add_activation_service(org.kde.ApperSentinel.service.in)
dbus_add_activation_service(org.freedesktop.PackageKit.service.in)

