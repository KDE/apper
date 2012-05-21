kde4_add_plugin(kded_apperd
    ApperdThread.cpp
    apperd.cpp
)

target_link_libraries(kded_apperd
    ${KDE4_KIO_LIBS}
    ${KDE4_SOLID_LIBS}
)

install(TARGETS kded_apperd DESTINATION ${PLUGIN_INSTALL_DIR})
