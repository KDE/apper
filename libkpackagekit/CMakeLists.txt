set(libkpk_SRCS
  KpkDelegate.cpp
  KpkTransaction.cpp
  KpkStrings.cpp
  KpkRepoSig.cpp
  KpkLicenseAgreement.cpp
  KpkPackageModel.cpp
  KpkSimplePackageModel.cpp
  KpkIcons.cpp
  KpkTransactionBar.cpp
  KpkProgressBar.cpp
)

kde4_add_ui_files(libkpk_SRCS
  KpkTransaction.ui
  KpkRepoSig.ui
  KpkLicenseAgreement.ui
)

kde4_add_library(kpackagekitlib SHARED ${libkpk_SRCS})
target_link_libraries(kpackagekitlib ${KDE4_KDEUI_LIBS} ${QPACKAGEKIT_LIB})

install(TARGETS kpackagekitlib ${INSTALL_TARGETS_DEFAULT_ARGS})
