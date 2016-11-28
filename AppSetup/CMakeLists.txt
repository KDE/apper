# CMakeLists for Apper AppSetup

pkg_check_modules(GLIB2 REQUIRED glib-2.0>=2.36)
pkg_check_modules(LIMBA REQUIRED limba>=0.2.0)
pkg_check_modules(APPSTREAM REQUIRED appstream>=0.7.6)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
        ${GLIB2_INCLUDE_DIR}
        ${LIMBA_INCLUDE_DIRS}
        ${APPSTREAM_INCLUDE_DIRS}
)

set(AppSetup_SRCS main.cpp
    SetupWizard.cpp
    SimplePage.cpp
)

ki18n_wrap_ui(AppSetup_SRCS
    SetupWizard.ui
    SimplePage.ui
)

add_executable(apper-appsetup ${AppSetup_SRCS})

set_target_properties(apper-appsetup PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR}/apper)

target_link_libraries(apper-appsetup
        ${KDE4_KDEUI_LIBS}
        ${GLIB2_LIBRARIES}
        ${LIMBA_LIBRARIES}
        ${APPSTREAM_LIBRARIES}
        apper_private
)

install(TARGETS apper-appsetup RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES apper_appsetup.desktop DESTINATION ${CMAKE_INSTALL_APPDIR})
