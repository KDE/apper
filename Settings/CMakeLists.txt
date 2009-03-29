set(kpk_settings_SRCS
    KcmKpkSettings.cpp
    KpkSettings.cpp
    KpkModelOrigin.cpp
)
kde4_add_ui_files(kpk_settings_SRCS
    KpkSettings.ui
)

kde4_add_plugin(kcm_kpk_settings ${kpk_settings_SRCS})
target_link_libraries(kcm_kpk_settings ${KDE4_KDEUI_LIBS} ${QPACKAGEKIT_LIBRARIES} kpackagekitlib)

install(TARGETS kcm_kpk_settings  DESTINATION ${PLUGIN_INSTALL_DIR} )
