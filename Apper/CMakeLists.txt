set(apper_SRCS
    MainUi.cpp
    BackendDetails.cpp
    Apper.cpp
    main.cpp
)

kde4_add_ui_files(apper_SRCS
    BackendDetails.ui
)

kde4_add_executable(apper_executable
    ${apper_SRCS}
)

target_link_libraries(apper_executable
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KUTILS_LIBS}
    ${QPACKAGEKIT2_LIBRARIES}
    apper
)

set_target_properties(apper_executable PROPERTIES INSTALL_RPATH ${LIB_INSTALL_DIR}/apper)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/apper_executable DESTINATION ${BIN_INSTALL_DIR} RENAME apper)
install(FILES apper.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES apper_installer.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES apper_settings.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES apper_updates.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES apper.appdata.xml DESTINATION share/appdata)

add_subdirectory(Icons)
add_subdirectory(Animations)
