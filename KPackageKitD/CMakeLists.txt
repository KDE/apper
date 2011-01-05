kde4_add_plugin(kded_kpackagekitd kpackagekitd.cpp)

target_link_libraries(kded_kpackagekitd
    ${KDE4_KIO_LIBS}
)

install(TARGETS kded_kpackagekitd DESTINATION ${PLUGIN_INSTALL_DIR})
