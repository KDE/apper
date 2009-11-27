find_package(KDE4Internal REQUIRED)

set(kpackagekit_smart_icon_SRCS
    main.cpp
    KPackageKitSmartIcon.cpp
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
)

qt4_add_dbus_adaptor(kpackagekit_smart_dbus_SRCS
    org.kde.KPackageKitSmartIcon.xml
    KpkInterface.h
    KpkInterface
)

qt4_add_dbus_adaptor(kpackagekit_smart_dbus_SRCS
    org.freedesktop.PackageKit.xml
    PkInterface.h
    PkInterface
)

kde4_add_executable(kpackagekitsmarticon
    ${kpackagekit_smart_icon_SRCS}
    ${kpackagekit_smart_dbus_SRCS}
)

target_link_libraries(kpackagekitsmarticon
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_SOLID_LIBS}
    ${QPACKAGEKIT_LIBRARIES}
    kpackagekitlib
)

install(TARGETS kpackagekitsmarticon DESTINATION ${LIBEXEC_INSTALL_DIR})

install(FILES KPackageKitSmartIcon.notifyrc DESTINATION ${DATA_INSTALL_DIR}/KPackageKitSmartIcon)

include(PkgConfigGetVar)
dbus_add_activation_service(org.kde.KPackageKitSmartIcon.service.in)