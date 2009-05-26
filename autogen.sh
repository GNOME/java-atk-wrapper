#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

REQUIRED_AUTOMAKE_VERSION=1.6

PKG_NAME=java-atk-wrapper
USE_GNOME2_MACROS=1 . gnome-autogen.sh
