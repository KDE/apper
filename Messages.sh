#! /usr/bin/env bash
$EXTRACTRC `find . -name "*.rc" -o -name "*.ui"` >> rc.cpp
$XGETTEXT `find . -name \*.cpp` -o $podir/apper.pot
rm -f rc.cpp

$XGETTEXT `find . -name \*.qml -o -name "*.cpp"` -o $podir/plasma_package_updater.pot
