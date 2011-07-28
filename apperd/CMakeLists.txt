kde4_add_plugin(kded_apperd apperd.cpp)

target_link_libraries(kded_apperd
    ${KDE4_KIO_LIBS}
)

install(TARGETS kded_apperd DESTINATION ${PLUGIN_INSTALL_DIR})
