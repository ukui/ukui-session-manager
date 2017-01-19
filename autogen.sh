#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

PKG_NAME="ukui-session-manager"

(test -f $srcdir/configure.ac) || {
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level $PKG_NAME directory"
    exit 1
}

which ukui-autogen || {
    echo "You need to install ukui-common from the ukui Git"
    exit 1
}

REQUIRED_AUTOMAKE_VERSION=1.9
USE_UKUI2_MACROS=1
USE_COMMON_DOC_BUILD=yes

. ukui-autogen

