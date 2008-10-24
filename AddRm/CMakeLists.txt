set(kpk_addrm_SRCS
    KcmKpkAddRm.cpp
    KpkAddRm.cpp
    KpkReviewChanges.cpp
    KpkRequirements.cpp
)

kde4_add_ui_files(kpk_addrm_SRCS
    KpkAddRm.ui
    KpkReviewChanges.ui
    KpkRequirements.ui
)

kde4_add_plugin(kcm_kpk_addrm ${kpk_addrm_SRCS})
target_link_libraries(kcm_kpk_addrm ${KDE4_KDEUI_LIBS} ${QPACKAGEKIT_LIB}
kpackagekitlib)

install(TARGETS kcm_kpk_addrm  DESTINATION ${PLUGIN_INSTALL_DIR} )
