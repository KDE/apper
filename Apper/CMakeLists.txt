file(GLOB QML_SRC qml/*.qml qml/*.js)

include_directories(${AppstreamQt_INCLUDES})

set(apper_SRCS
    ${QML_SRC}
#    MainUi.cpp
#    BackendDetails.cpp
    Apper.cpp
    main.cpp
)

qt5_add_resources(apper_SRCS
    application.qrc
    OPTIONS -compress 3 -threshold 3
)

add_executable(apper_executable
    ${apper_SRCS}
)

qt5_use_modules(apper_executable Quick DBus Widgets)

target_link_libraries(apper_executable
    ${PackageKitQt5_0_LIBRARIES}
    AppstreamQt
    apper
)

set_target_properties(apper_executable PROPERTIES INSTALL_RPATH ${LIB_INSTALL_DIR}/apper)

#install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/apper_executable DESTINATION ${BIN_INSTALL_DIR} RENAME apper)
#install(FILES apper.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
#install(FILES apper_installer.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
#install(FILES apper_settings.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
#install(FILES apper_updates.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

add_subdirectory(Icons)
add_subdirectory(Animations)
