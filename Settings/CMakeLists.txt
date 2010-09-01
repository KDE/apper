set(kpk_settings_SRCS
    KpkModelOrigin.cpp
    SettingsKCM.cpp
)
kde4_add_ui_files(kpk_settings_SRCS
    SettingsKCM.ui
)

kde4_add_plugin(kcm_kpk_settings ${kpk_settings_SRCS})
target_link_libraries(kcm_kpk_settings
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QPACKAGEKIT_LIBRARIES}
    kpackagekitlib)

install(TARGETS kcm_kpk_settings  DESTINATION ${PLUGIN_INSTALL_DIR} )
