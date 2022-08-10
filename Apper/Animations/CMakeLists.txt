# CMakeLists for animations used by Apper

file (GLOB _animations ${CMAKE_CURRENT_SOURCE_DIR}/*.png)
ecm_install_icons(ICONS ${_animations}
    DESTINATION ${KDE_INSTALL_DATADIR}/apper/icons)
