find_package(KDE4Internal REQUIRED)

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

kde4_create_manpage(man-apper.1.docbook 1 INSTALL_DESTINATION ${MAN_INSTALL_DIR})

add_subdirectory(Icons)
add_subdirectory(Animations)
