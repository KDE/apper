find_package(KDE4Internal REQUIRED)

set(kpackagekit_SRCS
    main.cpp
    KPackageKit.cpp
    KpkNotify.cpp
    KpkSmartUpdate.cpp
    KpkUi.cpp
    KpkInstallFiles.cpp
    ../Common/KpkStrings.cpp
    ../Common/KpkTransaction.cpp
    ../Common/KpkRepoSig.cpp
    ../Common/KpkLicenseAgreement.cpp
)

kde4_add_ui_files(kpackagekit_SRCS
    ../Common/KpkTransaction.ui
    ../Common/KpkRepoSig.ui
    ../Common/KpkLicenseAgreement.ui
)

kde4_add_executable(kpackagekit ${kpackagekit_SRCS})

target_link_libraries(kpackagekit ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} ${QPACKAGEKIT_LIB})

install(TARGETS kpackagekit ${INSTALL_TARGETS_DEFAULT_ARGS})

# keep the KPackageKit dir uppercased for proper name in notification
install(FILES KPackageKit.notifyrc DESTINATION ${DATA_INSTALL_DIR}/KPackageKit)

add_subdirectory( Icons )
