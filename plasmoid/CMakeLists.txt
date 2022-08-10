# CMakeLists for Apper plamoid

install(FILES package/metadata.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR} RENAME plasma-applet-org.packagekit.updater.desktop)
install(DIRECTORY package/ DESTINATION ${KDE_INSTALL_DATADIR}/plasma/plasmoids/org.packagekit.updater)
