
find_package(KDE4Internal REQUIRED)
find_package(GLIB2 REQUIRED)
pkg_check_modules(LISTALLER REQUIRED listaller-glib>=0.5.4)
pkg_check_modules(GEE REQUIRED gee-1.0>=0.6)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
		    ${GLIB2_INCLUDE_DIR}
		    ${GEE_INCLUDE_DIRS}
		    ${LISTALLER_INCLUDE_DIRS}
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
		      apper
)

kde4_create_manpage(man-apper-appsetup.1.docbook 1 INSTALL_DESTINATION ${MAN_INSTALL_DIR})

install(TARGETS apper-appsetup RUNTIME DESTINATION ${BIN_INSTALL_DIR})
