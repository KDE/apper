set(apper_SRCS
    MainUi.cpp
    BackendDetails.cpp
    Apper.cpp
    main.cpp
)

qt5_wrap_ui(apper_SRCS
    BackendDetails.ui
)

add_executable(apper
    ${apper_SRCS}
)

target_link_libraries(apper
    KF5::KCMUtils
    KF5::KDELibs4Support
    KF5::IconThemes
    ${PackageKitQt5_LIBRARIES}
    apper_private
)

set_target_properties(apper PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR}/apper)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/apper DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES apper.desktop DESTINATION ${CMAKE_INSTALL_APPDIR})
install(FILES apper_installer.desktop DESTINATION ${CMAKE_INSTALL_APPDIR})
install(FILES apper_settings.desktop DESTINATION ${CMAKE_INSTALL_APPDIR})
install(FILES apper_updates.desktop DESTINATION ${CMAKE_INSTALL_APPDIR})
install(FILES apper.appdata.xml DESTINATION ${CMAKE_INSTALL_METAINFODIR})

add_subdirectory(Icons)
add_subdirectory(Animations)
