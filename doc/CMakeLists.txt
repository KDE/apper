
kdoctools_create_manpage(man-apper.1.docbook 1 INSTALL_DESTINATION ${MAN_INSTALL_DIR})
if(LIMBA)
    kdoctools_create_manpage(man-apper-appsetup.1.docbook 1 INSTALL_DESTINATION ${MAN_INSTALL_DIR})
endif()
