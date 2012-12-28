set(updaterapplet_SRCS
    DBusUpdaterInterface.cpp
    updaterapplet.cpp
)

qt4_add_dbus_adaptor(updaterapplet_SRCS
    org.kde.ApperUpdaterIcon.xml
    DBusUpdaterInterface.h
    DBusUpdaterInterface
)

kde4_add_plugin(plasma_applet_updater ${updaterapplet_SRCS})

target_link_libraries(plasma_applet_updater
    ${KDE4_PLASMA_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTDECLARATIVE_LIBRARY}
    ${QPACKAGEKIT2_LIBRARIES}
    apper
)

set_target_properties(plasma_applet_updater PROPERTIES INSTALL_RPATH ${LIB_INSTALL_DIR}/apper)

install(TARGETS plasma_applet_updater DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-updater.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(DIRECTORY package/ DESTINATION ${DATA_INSTALL_DIR}/plasma/packages/org.packagekit.updater)
