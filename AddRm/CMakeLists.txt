set(kpk_addrm_SRCS
    KcmKpkAddRm.cpp
    KpkAddRm.cpp
    KpkAddRmModel.cpp
    KpkReviewChanges.cpp
    KpkRequirements.cpp
    ../Common/KpkDelegate.cpp
    ../Common/KpkTransaction.cpp
    ../Common/KpkStrings.cpp
    ../Common/KpkRepoSig.cpp
    ../Common/KpkLicenseAgreement.cpp
)

kde4_add_ui_files(kpk_addrm_SRCS
    KpkAddRm.ui
    KpkReviewChanges.ui
    KpkRequirements.ui
    ../Common/KpkTransaction.ui
    ../Common/KpkRepoSig.ui
    ../Common/KpkLicenseAgreement.ui
)

kde4_add_plugin(kcm_kpk_addrm ${kpk_addrm_SRCS})
target_link_libraries(kcm_kpk_addrm ${KDE4_KDEUI_LIBS} ${QPACKAGEKIT_LIB})

install(TARGETS kcm_kpk_addrm  DESTINATION ${PLUGIN_INSTALL_DIR} )
