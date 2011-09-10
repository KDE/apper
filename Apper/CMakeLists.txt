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

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/apper_executable DESTINATION ${BIN_INSTALL_DIR} RENAME apper)

add_subdirectory(Icons)
add_subdirectory(Animations)
