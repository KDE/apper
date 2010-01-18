set(kded_kpackagekitd_SRCS
    kpackagekitd.cpp)

kde4_add_plugin(kded_kpackagekitd ${kded_kpackagekitd_SRCS})

target_link_libraries(kded_kpackagekitd
    ${KDE4_KIO_LIBS}
    ${KDE4_SOLID_LIBS}
)

install(TARGETS kded_kpackagekitd DESTINATION ${PLUGIN_INSTALL_DIR})
