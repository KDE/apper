project(AppSetup)

find_package(KDE4Internal REQUIRED)
find_package(GLIB2 REQUIRED)
pkg_check_modules(LISTALLER REQUIRED listaller-glib>=0.5.4)
pkg_check_modules(GEE REQUIRED gee-1.0)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
		    ${GLIB2_INCLUDE_DIR}
		    ${GEE_INCLUDE_DIRS}
		    ${LISTALLER_INCLUDE_DIRS}
)

set(AppSetup_SRCS main.cpp
	SetupWizard.cpp
)

kde4_add_ui_files(AppSetup_SRCS
	SetupWizard.ui
)

kde4_add_executable(appsetup-kde ${AppSetup_SRCS})

target_link_libraries(appsetup-kde
		      ${KDE4_KDEUI_LIBS}
		      ${GLIB2_LIBRARIES}
		      ${LISTALLER_LIBRARIES}
		      apper
)
