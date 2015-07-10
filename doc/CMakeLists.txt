# CMakeLists for Apper documentation

kdoctools_create_manpage(man-apper.1.docbook 1 INSTALL_DESTINATION ${CMAKE_INSTALL_MANDIR})
if(LIMBA)
    kdoctools_create_manpage(man-apper-appsetup.1.docbook 1 INSTALL_DESTINATION ${CMAKE_INSTALL_MANDIR})
endif()
