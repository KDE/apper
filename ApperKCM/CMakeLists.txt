set(kpk_addrm_SRCS
    Settings/Settings.cpp
    Settings/OriginModel.cpp
    Updater/UpdateDetails.cpp
    Updater/DistroUpgrade.cpp
    Updater/CheckableHeader.cpp
    Updater/Updater.cpp
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
    Settings/Settings.ui
    Updater/UpdateDetails.ui
    Updater/Updater.ui
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
    ${QPACKAGEKIT2_LIBRARIES}
    ${KDE4_SOLID_LIBS}
    kpackagekitlib
)

install(TARGETS apper DESTINATION ${PLUGIN_INSTALL_DIR} )
