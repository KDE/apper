set(sentinel_SRCS
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
    ApperSentinel.cpp
    main.cpp
)

qt4_add_dbus_adaptor(sentinel_SRCS
    org.freedesktop.PackageKit.xml
    PkInterface.h
    PkInterface
)

kde4_add_ui_files(sentinel_SRCS
    IntroDialog.ui
    ReviewChanges.ui
    SessionTask.ui
)

kde4_add_executable(apper-sentinel
    ${sentinel_SRCS}
)

target_link_libraries(apper-sentinel
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QPACKAGEKIT2_LIBRARIES}
    ${QT_QTXMLPATTERNS_LIBRARY}
    apper
)

set_target_properties(apper-sentinel PROPERTIES INSTALL_RPATH ${LIB_INSTALL_DIR}/apper)

install(TARGETS apper-sentinel DESTINATION ${LIBEXEC_INSTALL_DIR})

install(FILES ApperSentinel.notifyrc DESTINATION ${DATA_INSTALL_DIR}/ApperSentinel)

include(MacroDBusAddActivationService)
dbus_add_activation_service(org.freedesktop.PackageKit.service.in)

