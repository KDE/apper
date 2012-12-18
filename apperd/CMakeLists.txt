set(kded_apperd_SRCS
    DistroUpgrade.cpp
    DBusInterface.cpp
    TransactionJob.cpp
    TransactionWatcher.cpp
    RefreshCacheTask.cpp
    Updater.cpp
    ApperdThread.cpp
    apperd.cpp
)

qt4_add_dbus_adaptor(kded_apperd_SRCS
    org.kde.apperd.xml
    DBusInterface.h
    DBusInterface
)

kde4_add_plugin(kded_apperd ${kded_apperd_SRCS})

target_link_libraries(kded_apperd
    ${KDE4_KIO_LIBS}
    ${KDE4_SOLID_LIBS}
    ${KDE4WORKSPACE_KWORKSPACE_LIBS}
    ${QPACKAGEKIT2_LIBRARIES}
    ${DEBCONF_KDE_LIB}
    apper
)

set_target_properties(kded_apperd PROPERTIES INSTALL_RPATH ${LIB_INSTALL_DIR}/apper)

install(TARGETS kded_apperd DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES apperd.notifyrc DESTINATION ${DATA_INSTALL_DIR}/apperd)
install(FILES apperd.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)
