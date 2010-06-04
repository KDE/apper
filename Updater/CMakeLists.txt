set(kpk_update_SRCS
    KpkUpdateDetails.cpp
    KpkDistroUpgrade.cpp
    KpkSimpleTransactionModel.cpp
    KpkHistory.cpp
    KpkTransactionFilterModel.cpp
    KcmKpkUpdate.cpp
)
kde4_add_ui_files(kpk_update_SRCS
    KpkUpdateDetails.ui
    KpkHistory.ui
    KcmKpkUpdate.ui
)

kde4_add_plugin(kcm_kpk_update ${kpk_update_SRCS})
target_link_libraries(kcm_kpk_update
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_SOLID_LIBS}
    ${QPACKAGEKIT_LIBRARIES}
    kpackagekitlib
)

install(TARGETS kcm_kpk_update DESTINATION ${PLUGIN_INSTALL_DIR} )
