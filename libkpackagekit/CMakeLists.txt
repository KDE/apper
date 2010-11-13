set(libkpk_SRCS
    AppInstall.cpp
    KpkIcons.cpp
    KpkStrings.cpp
    ApplicationLauncher.cpp
    ApplicationsDelegate.cpp
    CategoryDrawer.cpp
    KpkDelegate.cpp
    TransactionDelegate.cpp
    ProgressView.cpp
    KpkTransaction.cpp
    KpkRepoSig.cpp
    KpkLicenseAgreement.cpp
    KpkPackageModel.cpp
    KpkSimplePackageModel.cpp
    KpkProgressBar.cpp
    KpkReviewChanges.cpp
    KpkRequirements.cpp
    KpkAbstractIsRunning.cpp
    KpkImportance.cpp
    KpkSimulateModel.cpp
    KpkCategorizedView.cpp
)

kde4_add_ui_files(libkpk_SRCS
    ApplicationLauncher.ui
    KpkTransaction.ui
    KpkRepoSig.ui
    KpkLicenseAgreement.ui
    KpkReviewChanges.ui
    KpkRequirements.ui
)

kde4_add_library(kpackagekitlib SHARED ${libkpk_SRCS})
target_link_libraries(kpackagekitlib
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QPACKAGEKIT_LIBRARIES})

install(TARGETS kpackagekitlib ${INSTALL_TARGETS_DEFAULT_ARGS})
