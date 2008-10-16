find_package(KDE4Internal REQUIRED)

set(kpackagekit_smart_icon_SRCS
    main.cpp
    KPackageKitSmartIcon.cpp
    KpkTransactionTrayIcon.cpp
    ../Common/KpkStrings.cpp
    ../Common/KpkTransaction.cpp
    ../Common/KpkRepoSig.cpp
    ../Common/KpkLicenseAgreement.cpp
)

kde4_add_ui_files(kpackagekit_smart_icon_SRCS
    ../Common/KpkTransaction.ui
    ../Common/KpkRepoSig.ui
    ../Common/KpkLicenseAgreement.ui
)

kde4_add_executable(kpackagekit-smart-icon ${kpackagekit_smart_icon_SRCS})

target_link_libraries(kpackagekit-smart-icon ${KDE4_KDEUI_LIBS} ${QPACKAGEKIT_LIB})

install(TARGETS kpackagekit-smart-icon ${INSTALL_TARGETS_DEFAULT_ARGS})
