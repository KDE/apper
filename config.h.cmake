#ifndef CONFIG_H
#define CONFIG_H

// Define if you have DebconfKDE libraries and header files.
#cmakedefine HAVE_DEBCONFKDE

// Define if your backend have autoremove feature.
#cmakedefine HAVE_AUTOREMOVE

// Define if AppStream data is available.
#cmakedefine HAVE_APPSTREAM

// Define the AppStream categories path.
#cmakedefine AS_CATEGORIES_PATH "@AS_CATEGORIES_PATH@"

// Define the edit origins command.
#cmakedefine EDIT_ORIGNS_DESKTOP_NAME "@EDIT_ORIGNS_DESKTOP_NAME@"

// Define the Apper version.
#cmakedefine APP_VERSION "@APP_VERSION@"

#endif //CONFIG_H
