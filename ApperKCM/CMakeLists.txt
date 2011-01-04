set(kpk_addrm_SRCS
    Settings/SettingsKCM.cpp
    Settings/KpkModelOrigin.cpp
    KpkFiltersMenu.cpp
    ClickableLabel.cpp
    ScreenShotViewer.cpp
    PackageDetails.cpp
    GraphicsOpacityDropShadowEffect.cpp
    CategoryModel.cpp
    BrowseView.cpp
    TransactionModel.cpp
    TransactionFilterModel.cpp
    TransactionHistory.cpp
    ApperKCM.cpp
)

kde4_add_ui_files(kpk_addrm_SRCS
    Settings/SettingsKCM.ui
    PackageDetails.ui
    BrowseView.ui
    TransactionHistory.ui
    ApperKCM.ui
)

kde4_add_plugin(apper ${kpk_addrm_SRCS})
target_link_libraries(apper
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KUTILS_LIBS}
    ${QPACKAGEKIT_LIBRARIES}
    kpackagekitlib
)

install(TARGETS apper DESTINATION ${PLUGIN_INSTALL_DIR} )
