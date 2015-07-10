# CMakeLists for Apper QML plugin

set(apper_qml_plugin_SRCS
   DBusUpdaterInterface.cpp
   daemonhelper.cpp
   qmlplugins.cpp
)

qt5_add_dbus_adaptor(apper_qml_plugin_SRCS
    org.kde.ApperUpdaterIcon.xml
    DBusUpdaterInterface.h
    DBusUpdaterInterface
)

add_library(apperplugin SHARED ${apper_qml_plugin_SRCS})
target_link_libraries(apperplugin
    ${QT_QTDECLARATIVE_LIBRARY}
    KF5::WidgetsAddons
    ${PackageKitQt5_LIBRARIES}
    apper_private
)

qt5_use_modules(apperplugin Quick Core)

set_target_properties(apperplugin PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR}/apper)

install(TARGETS apperplugin DESTINATION ${CMAKE_INSTALL_QTQUICKIMPORTSDIR}/org/kde/apper)
install(FILES qmldir DESTINATION ${CMAKE_INSTALL_QTQUICKIMPORTSDIR}/org/kde/apper)
