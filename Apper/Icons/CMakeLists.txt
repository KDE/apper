
file (GLOB _icons ${CMAKE_CURRENT_SOURCE_DIR}/*.png)
ecm_install_icons(ICONS ${_icons}
    DESTINATION ${DATA_INSTALL_DIR}/apper/icons)
