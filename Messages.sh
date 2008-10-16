#! /usr/bin/env bash
$EXTRACTRC `find . -name "*.rc" -o -name "*.ui"` > rc.cpp
$XGETTEXT `find . -name \*.cpp` -o $podir/kpackagekit.pot
rm -f rc.cpp

