#ifndef CONFIG_H
#define CONFIG_H

// Define if you have DebconfKDE libraries and header files.
#cmakedefine HAVE_DEBCONFKDE

// Define if your backend have autoremove feature.
#cmakedefine HAVE_AUTOREMOVE

// Define if app-install data is available.
#cmakedefine HAVE_APPINSTALL

// Define the app-install database path.
#cmakedefine AI_DB_PATH "@AI_DB_PATH@"

// Define the app-install categories path.
#cmakedefine AI_CATEGORIES_PATH "@AI_CATEGORIES_PATH@"

// Define the edit origins command.
#cmakedefine EDIT_ORIGNS_DESKTOP_NAME "@EDIT_ORIGNS_DESKTOP_NAME@"

#endif //CONFIG_H