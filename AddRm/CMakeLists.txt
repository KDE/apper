set(kpk_addrm_SRCS
    KpkFiltersMenu.cpp
    KpkPackageDetails.cpp
    KpkAddRm.cpp
    KcmKpkAddRm.cpp
)

kde4_add_ui_files(kpk_addrm_SRCS
    KpkPackageDetails.ui
    KpkAddRm.ui
)

kde4_add_plugin(kcm_kpk_addrm ${kpk_addrm_SRCS})
target_link_libraries(kcm_kpk_addrm
    ${KDE4_KDEUI_LIBS}
    ${QPACKAGEKIT_LIBRARIES}
    kpackagekitlib
)

install(TARGETS kcm_kpk_addrm  DESTINATION ${PLUGIN_INSTALL_DIR} )
