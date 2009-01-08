find_package(KDE4Internal REQUIRED)

set(kpackagekit_smart_icon_SRCS
    main.cpp
    KPackageKitSmartIcon.cpp
    KpkTransactionTrayIcon.cpp
    KpkUpdateIcon.cpp
)

kde4_add_executable(kpackagekit-smart-icon ${kpackagekit_smart_icon_SRCS})

target_link_libraries(kpackagekit-smart-icon ${KDE4_KDEUI_LIBS} ${KDE4_KUTILS_LIBS} ${QPACKAGEKIT_LIB} kpackagekitlib)

install(TARGETS kpackagekit-smart-icon ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES kpackagekit-smart-icon.notifyrc DESTINATION ${DATA_INSTALL_DIR}/kpackagekit-smart-icon)
