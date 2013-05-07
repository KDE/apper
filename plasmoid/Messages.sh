#! /usr/bin/env bash
$XGETTEXT `find . -name \*.cpp` -o $podir/plasma_applet_updater.pot
$XGETTEXT `find package -name '*.qml'` -L Java -o $podir/plasma_applet_org.packagekit.updater.pot
