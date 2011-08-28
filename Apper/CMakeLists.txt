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

kde4_add_executable(apper
    ${apper_SRCS}
)

target_link_libraries(apper
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KUTILS_LIBS}
    ${QPACKAGEKIT2_LIBRARIES}
    Apper
)

install(TARGETS apper ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(Icons)
add_subdirectory(Animations)
