#Set the correct compiler options
IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
   # 32 bit
   MESSAGE(STATUS "Apper detected that you use a 32 bit processor.")
ELSE(CMAKE_SIZEOF_VOID_P EQUAL 4)
   # 64 bit (well, anything else than 32 bit, but someone use something else than 32 or 64 bit ?)
   set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
   MESSAGE(STATUS "Apper detected that you use a 64 bit processor. Added -fPIC to the CXX_FLAGS.")
ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 4)

set(libapper_SRCS
    AppInstall.cpp
    PkIcons.cpp
    PkStrings.cpp
    ApplicationLauncher.cpp
    ApplicationsDelegate.cpp
    CategoryDrawer.cpp
    ChangesDelegate.cpp
    TransactionDelegate.cpp
    ProgressView.cpp
    PkTransaction.cpp
    PkTransactionDialog.cpp
    RepoSig.cpp
    LicenseAgreement.cpp
    PackageModel.cpp
    CustomProgressBar.cpp
    Requirements.cpp
    PackageImportance.cpp
    SimulateModel.cpp
    CategorizedView.cpp
)

kde4_add_ui_files(libapper_SRCS
    ApplicationLauncher.ui
    PkTransaction.ui
    RepoSig.ui
    LicenseAgreement.ui
    Requirements.ui
)

kde4_add_library(apper SHARED ${libapper_SRCS})

target_link_libraries(apper
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QPACKAGEKIT2_LIBRARIES}
)

install(TARGETS apper DESTINATION ${LIB_INSTALL_DIR}/apper)
