pkg_check_modules(GLIB2 REQUIRED glib-2.0>=2.36)
pkg_check_modules(LISTALLER REQUIRED listaller-glib>=0.5.8)
pkg_check_modules(APPSTREAM REQUIRED appstream>=0.6.2)
pkg_check_modules(GEE REQUIRED gee-1.0>=0.6)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
		    ${GLIB2_INCLUDE_DIR}
		    ${GEE_INCLUDE_DIRS}
		    ${LISTALLER_INCLUDE_DIRS}
		    ${APPSTREAM_INCLUDE_DIRS}
)

set(AppSetup_SRCS main.cpp
	SetupWizard.cpp
	SimplePage.cpp
	LicensePage.cpp
)

kde4_add_ui_files(AppSetup_SRCS
	SetupWizard.ui
	SimplePage.ui
	LicensePage.ui
)

kde4_add_executable(apper-appsetup ${AppSetup_SRCS})

set_target_properties(apper-appsetup PROPERTIES INSTALL_RPATH ${LIB_INSTALL_DIR}/apper)

target_link_libraries(apper-appsetup
		      ${KDE4_KDEUI_LIBS}
		      ${GLIB2_LIBRARIES}
		      ${LISTALLER_LIBRARIES}
		      ${APPSTREAM_LIBRARIES}
		      apper
)

install(TARGETS apper-appsetup RUNTIME DESTINATION ${BIN_INSTALL_DIR})
install(FILES apper_appsetup.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
