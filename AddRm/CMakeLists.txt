set(kpk_addrm_SRCS
    KpkFiltersMenu.cpp
    KpkPackageDetails.cpp
    ApplicationsDelegate.cpp
    CategoryModel.cpp
    BrowseView.cpp
    AddRmKCM.cpp
)

kde4_add_ui_files(kpk_addrm_SRCS
    KpkPackageDetails.ui
    BrowseView.ui
    AddRmKCM.ui
)

kde4_add_plugin(kcm_kpk_addrm ${kpk_addrm_SRCS})
target_link_libraries(kcm_kpk_addrm
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QPACKAGEKIT_LIBRARIES}
    kpackagekitlib
)

install(TARGETS kcm_kpk_addrm  DESTINATION ${PLUGIN_INSTALL_DIR} )
