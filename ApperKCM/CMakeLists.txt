set(kcm_apper_SRCS
    Settings/Settings.cpp
    Settings/OriginModel.cpp
    Updater/UpdateDetails.cpp
    Updater/DistroUpgrade.cpp
    Updater/CheckableHeader.cpp
    Updater/Updater.cpp
    FiltersMenu.cpp
    ClickableLabel.cpp
    ScreenShotViewer.cpp
    PackageDetails.cpp
    GraphicsOpacityDropShadowEffect.cpp
    CategoryModel.cpp
    BrowseView.cpp
    TransactionModel.cpp
    TransactionFilterModel.cpp
    TransactionHistory.cpp
    ApperKCM.cpp
)

qt5_wrap_ui(kcm_apper_SRCS
    Settings/Settings.ui
    Updater/UpdateDetails.ui
    Updater/Updater.ui
    PackageDetails.ui
    BrowseView.ui
    TransactionHistory.ui
    ApperKCM.ui
)

add_library(kcm_apper MODULE ${kcm_apper_SRCS})

target_link_libraries(kcm_apper
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KUTILS_LIBS}
    ${PackageKitQt4_LIBRARIES}
    ${KDE4_SOLID_LIBS}
    apper
)

set_target_properties(kcm_apper PROPERTIES INSTALL_RPATH ${LIB_INSTALL_DIR}/apper)

install(TARGETS kcm_apper DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_apper.desktop DESTINATION ${SERVICES_INSTALL_DIR})
