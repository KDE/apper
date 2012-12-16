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

qt4_add_dbus_adaptor(pksession_SRCS
    org.freedesktop.PackageKit.xml
    PkInterface.h
    PkInterface
)

kde4_add_ui_files(pksession_SRCS
    IntroDialog.ui
    ReviewChanges.ui
    SessionTask.ui
)

kde4_add_executable(apper-pk-session
    ${pksession_SRCS}
)

target_link_libraries(apper-pk-session
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QPACKAGEKIT2_LIBRARIES}
    ${QT_QTXMLPATTERNS_LIBRARY}
    apper
)

set_target_properties(apper-pk-session PROPERTIES INSTALL_RPATH ${LIB_INSTALL_DIR}/apper)

install(TARGETS apper-pk-session DESTINATION ${LIBEXEC_INSTALL_DIR})

include(MacroDBusAddActivationService)
dbus_add_activation_service(org.freedesktop.PackageKit.service.in)

