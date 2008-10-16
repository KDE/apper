set(kpk_update_SRCS
    KcmKpkUpdate.cpp
    KpkUpdate.cpp
    KpkUpdateModel.cpp
    ../Common/KpkStrings.cpp
    ../Common/KpkDelegate.cpp
    ../Common/KpkTransaction.cpp
    ../Common/KpkRepoSig.cpp
    ../Common/KpkLicenseAgreement.cpp
)
kde4_add_ui_files(kpk_update_SRCS
    KpkUpdate.ui
    ../Common/KpkTransaction.ui
    ../Common/KpkRepoSig.ui
    ../Common/KpkLicenseAgreement.ui
)

kde4_add_plugin(kcm_kpk_update ${kpk_update_SRCS})
target_link_libraries(kcm_kpk_update ${KDE4_KDEUI_LIBS} ${QPACKAGEKIT_LIB})

install(TARGETS kcm_kpk_update DESTINATION ${PLUGIN_INSTALL_DIR} )
