project(AppSetup)

find_package(KDE4Internal REQUIRED)
find_package(GLIB2 REQUIRED)
pkg_check_modules(LISTALLER REQUIRED listaller-glib>=0.5.4)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
		    ${GLIB2_INCLUDE_DIR}
		    ${LISTALLER_INCLUDE_DIRS}
)

set(AppSetup_SRCS SetupWizard.cpp main.cpp)

kde4_add_executable(appsetup-kde ${AppSetup_SRCS})

target_link_libraries(appsetup-kde
		      ${KDE4_KDEUI_LIBS}
		      ${GLIB2_LIBRARIES}
		      ${LISTALLER_LIBRARIES}
		      apper
)
