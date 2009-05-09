find_package(KDE4Internal REQUIRED)

set(kpackagekit_smart_icon_SRCS
    main.cpp
    KPackageKitSmartIcon.cpp
    KpkTransactionTrayIcon.cpp
    KpkUpdateIcon.cpp
    KpkDistroUpgrade.cpp
    KpkInterface.cpp
    KpkTransactionWatcher.cpp
)

qt4_add_dbus_adaptor(kpackagekit_smart_dbus_SRCS
    org.kde.KPackageKitSmartIcon.xml
    KpkInterface.h
    KpkInterface
)

kde4_add_executable(kpackagekit-smart-icon
    ${kpackagekit_smart_icon_SRCS}
    ${kpackagekit_smart_dbus_SRCS}
)

target_link_libraries(kpackagekit-smart-icon
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KUTILS_LIBS}
    ${QPACKAGEKIT_LIBRARIES}
    ${KDE4_SOLID_LIBS}
    kpackagekitlib
)

install(TARGETS kpackagekit-smart-icon DESTINATION ${LIBEXEC_INSTALL_DIR})

install(FILES KPackageKitSmartIcon.notifyrc DESTINATION ${DATA_INSTALL_DIR}/KPackageKit)

include(PkgConfigGetVar)
dbus_add_activation_service(org.kde.KPackageKitSmartIcon.service.in)