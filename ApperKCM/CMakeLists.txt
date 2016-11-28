# CMakeLists for the Apper KDE control module (KCM)

add_definitions(-DTRANSLATION_DOMAIN=\"apper\")

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

ki18n_wrap_ui(kcm_apper_SRCS
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
    KF5::WidgetsAddons
    KF5::KIOFileWidgets
    KF5::KCMUtils
    KF5::KDELibs4Support
    KF5::IconThemes
    ${PackageKitQt5_LIBRARIES}
    apper_private
)

set_target_properties(kcm_apper PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR}/apper)

install(TARGETS kcm_apper DESTINATION ${CMAKE_INSTALL_QTPLUGINDIR})
install(FILES kcm_apper.desktop DESTINATION ${CMAKE_INSTALL_KSERVICES5DIR})
