find_package(KDE4Internal REQUIRED)

set(sentinel_SRCS
    StatusNotifierItem.cpp
    TransactionTrayIcon.cpp
    TransactionJob.cpp
    KpkTransactionTrayIcon.cpp
    KpkUpdateIcon.cpp
    KpkDistroUpgrade.cpp
    KpkInterface.cpp
    KpkTransactionWatcher.cpp
    PkInterface.cpp
    KpkAbstractTask.cpp
    PkInstallPackageNames.cpp
    PkSearchFile.cpp
    PkIsInstalled.cpp
    PkInstallMimeTypes.cpp
    PkInstallGStreamerResources.cpp
    PkInstallFontconfigResources.cpp
    PkInstallPackageFiles.cpp
    PkInstallProvideFiles.cpp
    PkInstallCatalogs.cpp
    PkRemovePackageByFiles.cpp
    PkInstallPrinterDrivers.cpp
    KPackageKitSmartIcon.cpp
    main.cpp
)

qt4_add_dbus_adaptor(sentinel_dbus_SRCS
    org.kde.ApperSentinel.xml
    KpkInterface.h
    KpkInterface
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
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_SOLID_LIBS}
    ${QPACKAGEKIT2_LIBRARIES}
    ${DEBCONF_KDE_LIB}
    sharedlib
)

install(TARGETS apper-sentinel DESTINATION ${LIBEXEC_INSTALL_DIR})

install(FILES ApperSentinel.notifyrc DESTINATION ${DATA_INSTALL_DIR}/ApperSentinel)

include(PkgConfigGetVar)
dbus_add_activation_service(org.kde.ApperSentinel.service.in)
dbus_add_activation_service(org.freedesktop.PackageKit.service.in)

