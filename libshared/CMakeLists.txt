set(libshared_SRCS
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
    KpkAbstractIsRunning.cpp
    KpkImportance.cpp
    KpkSimulateModel.cpp
    KpkCategorizedView.cpp
)

kde4_add_ui_files(libshared_SRCS
    ApplicationLauncher.ui
    PkTransaction.ui
    KpkRepoSig.ui
    KpkLicenseAgreement.ui
    KpkReviewChanges.ui
    KpkRequirements.ui
)

kde4_add_library(sharedlib STATIC ${libshared_SRCS})
target_link_libraries(sharedlib
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QPACKAGEKIT2_LIBRARIES})
