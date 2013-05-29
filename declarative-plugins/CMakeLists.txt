set(apper_qml_plugin_SRCS
   DBusUpdaterInterface.cpp
   daemonhelper.cpp
   qmlplugins.cpp
)

qt4_add_dbus_adaptor(apper_qml_plugin_SRCS
    org.kde.ApperUpdaterIcon.xml
    DBusUpdaterInterface.h
    DBusUpdaterInterface
)

kde4_add_library(apperplugin SHARED ${apper_qml_plugin_SRCS})
target_link_libraries(apperplugin
    ${QT_QTDECLARATIVE_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    ${QPACKAGEKIT2_LIBRARIES}
    apper
)

set_target_properties(apperplugin PROPERTIES INSTALL_RPATH ${LIB_INSTALL_DIR}/apper)

install(TARGETS apperplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/apper)
install(FILES qmldir DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/apper)
