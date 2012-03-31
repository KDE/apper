project(AppSetup)

find_package(KDE4Internal REQUIRED)
find_package(GLIB2 REQUIRED)
pkg_check_modules(LISTALLER REQUIRED listaller-glib>=0.5.4)
pkg_check_modules(GEE REQUIRED gee>=0.6)

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

kde4_add_executable(appsetup-kde ${AppSetup_SRCS})

set_target_properties(appsetup-kde PROPERTIES INSTALL_RPATH ${LIB_INSTALL_DIR}/apper)

target_link_libraries(appsetup-kde
		      ${KDE4_KDEUI_LIBS}
		      ${GLIB2_LIBRARIES}
		      ${LISTALLER_LIBRARIES}
		      apper
)

install(TARGETS appsetup-kde RUNTIME DESTINATION ${BIN_INSTALL_DIR})
