# CMakeLists for animations used by Apper

file (GLOB _animations ${CMAKE_CURRENT_SOURCE_DIR}/*.png)
ecm_install_icons(ICONS ${_animations}
    DESTINATION ${DATA_INSTALL_DIR}/apper/icons)
