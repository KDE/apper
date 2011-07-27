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
    KpkIcons.cpp
    KpkStrings.cpp
    ApplicationLauncher.cpp
    ApplicationsDelegate.cpp
    CategoryDrawer.cpp
    KpkDelegate.cpp
    TransactionDelegate.cpp
    ProgressView.cpp
    PkTransaction.cpp
    PkTransactionDialog.cpp
    KpkRepoSig.cpp
    KpkLicenseAgreement.cpp
    KpkPackageModel.cpp
    KpkProgressBar.cpp
    KpkReviewChanges.cpp
    KpkRequirements.cpp
    KpkImportance.cpp
    KpkSimulateModel.cpp
    KpkCategorizedView.cpp
)

kde4_add_ui_files(libapper_SRCS
    ApplicationLauncher.ui
    PkTransaction.ui
    KpkRepoSig.ui
    KpkLicenseAgreement.ui
    KpkReviewChanges.ui
    KpkRequirements.ui
)

kde4_add_library(sharedlib SHARED ${libapper_SRCS})
target_link_libraries(sharedlib
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QPACKAGEKIT2_LIBRARIES})

install(TARGETS sharedlib ${INSTALL_TARGETS_DEFAULT_ARGS})
