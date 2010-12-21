find_package(KDE4Internal REQUIRED)

set(kpackagekit_SRCS
    MainUi.cpp
    BackendDetails.cpp
    Apper.cpp
    main.cpp
)

kde4_add_ui_files(kpackagekit_SRCS
    BackendDetails.ui
)

kde4_add_executable(apper
    ${kpackagekit_SRCS}
)

target_link_libraries(apper
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KUTILS_LIBS}
    ${QPACKAGEKIT_LIBRARIES}
    kpackagekitlib
)

install(TARGETS apper ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(Icons)
add_subdirectory(Animations)