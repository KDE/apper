#! /usr/bin/env bash
$EXTRACTRC `find . -name "*.rc" -o -name "*.ui"` >> rc.cpp
$XGETTEXT `find . -name \*.cpp | grep -v '/plasmoid/'` -o $podir/apper.pot
rm -f rc.cpp
