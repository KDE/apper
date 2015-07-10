# CMakeLists for Apper private shared library

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
    pkg_check_modules(GLIB2 REQUIRED glib-2.0>=2.36)
    pkg_check_modules(APPSTREAM REQUIRED appstream>=0.8.2)

    set(libapper_SRCS ${libapper_SRCS} AppStream.cpp)
endif()

qt5_wrap_ui(libapper_SRCS
    ApplicationLauncher.ui
    PkTransactionWidget.ui
    RepoSig.ui
    LicenseAgreement.ui
    Requirements.ui
    InfoWidget.ui
)

add_library(apper_private SHARED ${libapper_SRCS})

include_directories(${CMAKE_CURRENT_BINARY_DIR}
    ${GLIB2_INCLUDE_DIR}
    ${APPSTREAM_INCLUDE_DIRS}
)

target_link_libraries(apper_private
    KF5::WidgetsAddons
    KF5::KIOFileWidgets
    KF5::KDELibs4Support
    KF5::IconThemes
    ${QT_QTSQL_LIBRARY}
    ${PackageKitQt5_LIBRARIES}
    ${APPSTREAM_LIBRARIES}
)

qt5_use_modules(apper_private Quick Core Sql)

install(TARGETS apper_private DESTINATION ${CMAKE_INSTALL_LIBDIR}/apper)
