# CMakeLists for Apper private shared library

add_definitions(-DTRANSLATION_DOMAIN=\"apper\")

# Set the correct compiler options
IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
   # 32 bit
   MESSAGE(STATUS "Apper detected that you use a 32 bit processor.")
ELSE(CMAKE_SIZEOF_VOID_P EQUAL 4)
   # 64 bit (well, anything else than 32 bit, but someone use something else than 32 or 64 bit ?)
   set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
   MESSAGE(STATUS "Apper detected that you use a 64 bit processor. Added -fPIC to the CXX_FLAGS.")
ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 4)

set(libapper_SRCS
    CategoryMatcher.cpp
    PkIcons.cpp
    PkStrings.cpp
    ApplicationLauncher.cpp
    ApplicationsDelegate.cpp
    ApplicationSortFilterModel.cpp
    CategoryDrawer.cpp
    ChangesDelegate.cpp
    TransactionDelegate.cpp
    PkTransaction.cpp
    PkTransactionWidget.cpp
    PkTransactionProgressModel.cpp
    RepoSig.cpp
    LicenseAgreement.cpp
    PackageModel.cpp
    CustomProgressBar.cpp
    Requirements.cpp
    PackageImportance.cpp
    CategorizedView.cpp
    InfoWidget.cpp
)

if(APPSTREAM)
    find_package(AppStreamQt REQUIRED)

    set(libapper_SRCS ${libapper_SRCS} AppStream.cpp)
endif()

ki18n_wrap_ui(libapper_SRCS
    ApplicationLauncher.ui
    PkTransactionWidget.ui
    RepoSig.ui
    LicenseAgreement.ui
    Requirements.ui
    InfoWidget.ui
)

add_library(apper_private SHARED ${libapper_SRCS})

target_link_libraries(apper_private
    KF5::WidgetsAddons
    KF5::KIOFileWidgets
    KF5::IconThemes
    KF5::I18n
    Qt5::Core
    PK::packagekitqt5
    AppStreamQt
)

install(TARGETS apper_private DESTINATION ${CMAKE_INSTALL_LIBDIR}/apper)
