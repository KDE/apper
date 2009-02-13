set(kpk_update_SRCS
    KcmKpkUpdate.cpp
    KpkUpdate.cpp
    KpkUpdateDetails.cpp
    KpkDistroUpgrade.cpp
)
kde4_add_ui_files(kpk_update_SRCS
    KpkUpdate.ui
    KpkUpdateDetails.ui
)

kde4_add_plugin(kcm_kpk_update ${kpk_update_SRCS})
target_link_libraries(kcm_kpk_update ${KDE4_KDEUI_LIBS} ${QPACKAGEKIT_LIB} ${KDE4_SOLID_LIBS} kpackagekitlib)

install(TARGETS kcm_kpk_update DESTINATION ${PLUGIN_INSTALL_DIR} )
