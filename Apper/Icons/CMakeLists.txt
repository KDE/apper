# CMakeLists for additional icons

file (GLOB _icons ${CMAKE_CURRENT_SOURCE_DIR}/*.png)
ecm_install_icons(ICONS ${_icons}
    DESTINATION ${KDE_INSTALL_DATADIR}/apper/icons)
