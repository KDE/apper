# CMakeLists for the kded component

set(kded_apperd_SRCS
    DistroUpgrade.cpp
    DBusInterface.cpp
    TransactionJob.cpp
    TransactionWatcher.cpp
    RefreshCacheTask.cpp
    Updater.cpp
    RebootListener.cpp
    ApperdThread.cpp
    apperd.cpp
)

qt5_add_dbus_adaptor(kded_apperd_SRCS
    org.kde.apperd.xml
    DBusInterface.h
    DBusInterface
)

add_library(kded_apperd MODULE ${kded_apperd_SRCS})

target_link_libraries(kded_apperd
    KF5::WidgetsAddons
    KF5::KIOFileWidgets
    KF5::Notifications
    KF5::DBusAddons
    PW::KWorkspace
    PK::packagekitqt5
    apper_private
)

if(DEBCONF_SUPPORT)
    target_link_libraries(kded_apperd DebconfKDE::Main)
endif()

set_target_properties(kded_apperd PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR}/apper)

install(TARGETS kded_apperd DESTINATION ${CMAKE_INSTALL_QTPLUGINDIR})
install(FILES apperd.notifyrc DESTINATION ${CMAKE_INSTALL_DATADIR}/apperd)
install(FILES apperd.desktop DESTINATION ${CMAKE_INSTALL_KSERVICES5DIR}/kded)
