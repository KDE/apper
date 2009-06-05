find_package(KDE4Internal REQUIRED)

set(kpackagekit_SRCS
    main.cpp
    KPackageKit.cpp
    KpkInstallFiles.cpp
    KpkInstallMimeType.cpp
    KpkInstallPackageName.cpp
    KpkInstallProvideFile.cpp
    KpkRemovePackageByFile.cpp
    KpkBackendDetails.cpp
    KpkMainUi.cpp
)

kde4_add_ui_files(kpackagekit_SRCS
    KpkBackendDetails.ui
)

kde4_add_executable(kpackagekit
    ${kpackagekit_SRCS}
)

target_link_libraries(kpackagekit
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KUTILS_LIBS}
    ${QPACKAGEKIT_LIBRARIES}
    kpackagekitlib
)

install(TARGETS kpackagekit ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(Icons)
add_subdirectory(Animations)