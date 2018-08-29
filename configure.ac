AC_PREREQ(2.63)
AC_INIT([java-atk-wrapper],[0.33.3],[https://bugzilla.gnome.org/enter_bug.cgi?product=java-atk-wrapper])
: ${CXXFLAGS=""}

AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE([1.11 -Wall check-news])

AM_SILENT_RULES([yes])
m4_pattern_allow([AM_PROG_AR])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

# Before making a release, the LT_VERSION string should be modified.
# The string is of the form C:R:A.
# - If interfaces have been changed or added, but binary compatibility has
#   been preserved, change to C+1:0:A+1
# - If binary compatibility has been broken (eg removed or changed interfaces)
#   change to C+1:0:0
# - If the interface is the same as the previous version, change to C:R+1:A

LIB_ATK_WRAPPER_LT_VERSION=6:0:0
AC_SUBST(LIB_ATK_WRAPPER_LT_VERSION)

# jni
AC_CONFIG_HEADERS([config.h])

AC_PROG_CC(gcc)
AC_PROG_CXX
AC_PROG_INSTALL
AC_PATH_XTRA

# Initialize libtool
LT_PREREQ([2.2.6])
LT_INIT([disable-static])

AC_CONFIG_MACRO_DIR([m4])

m4_define([req_ver_atk], [1.18.0])

PKG_CHECK_MODULES(JAW, [
  atk >= 2.14.0
  atk-bridge-2.0
  atspi-2 >= 2.14.0
  dbus-1
  glib-2.0 >= 2.32.0
  gthread-2.0
  gdk-2.0
  gdk-3.0
  gobject-2.0
])
AC_SUBST(JAW_LIBS)

case "${target_os}" in
cygwin*)
  LIB_ATK_WRAPPER_LD_FLAGS="-avoid-version -no-undefined"
  ;;
*)
  LIB_ATK_WRAPPER_LD_FLAGS="-version-info $LIB_ATK_WRAPPER_LT_VERSION -no-undefined"
  ;;
esac
AC_SUBST(LIB_ATK_WRAPPER_LD_FLAGS)

# java wrapper

AM_CONDITIONAL(USER, test `whoami` = "root")
JAVA_REQUIRED=1.6
JAVA_ERROR_MESSAGE="Java $JAVA_REQUIRED or later is required to build java-access-bridge"

AC_ARG_VAR([JAVA_HOME],[Java Runtime Environment location])
AC_ARG_VAR([JDK_SRC],[JDK installation path])

CUR_DIR=`pwd`
SRC_DIR=`dirname $0`

AC_ARG_WITH([jdk_auto_detect],
            [AS_HELP_STRING([--with-jdk-auto-detect],
            [Auto-detection of jdk location])],
            [],
            [with_jdk_auto_detect=yes])
if test "x$with_jdk_auto_detect" = "xyes"; then
  `javac $JAVACFLAGS -classpath $CUR_DIR/wrapper/org/GNOME/Accessibility $CUR_DIR/JavaInfo.java`
  JDK_SRC=`java JavaInfo`
  JAVA_HOME=`java JavaInfo`/jre
else
  if test -z "$JDK_SRC"; then
    AC_MSG_ERROR("You should specify JDK_SRC")
  fi
fi

AC_SUBST(JDK_SRC)
AC_SUBST(JAVA_HOME)

if test -z "$JAVA_HOME"; then
  JAVA_PATH=${PATH}
else
  JAVA_PATH=${JAVA_HOME}:${JAVA_HOME}/jre/bin:${JAVA_HOME}/bin:${JAVA_HOME}/../bin:${PATH}
fi

AC_PATH_PROG(JAVA,java,,${JAVA_PATH})
if test -z "$JAVA"; then
  AC_MSG_ERROR([$JAVA_ERROR_MESSAGE])
fi
AC_SUBST(JAVA)

AC_PATH_PROG(JAVAC,javac,,${JAVA_PATH})
if test -z "$JAVAC"; then
  AC_MSG_ERROR([$JAVA_ERROR_MESSAGE])
fi
AC_SUBST(JAVAC)

AC_PATH_PROG(JAVAH,javah,,${JAVA_PATH})
if test -z "$JAVAH"; then
  AC_MSG_ERROR([$JAVA_ERROR_MESSAGE])
fi
AC_SUBST(JAVAH)

AC_SUBST(JAVACFLAGS)

AC_PATH_PROG(JAR,jar,,${JAVA_PATH})
if test -z "$JAR"; then
  AC_MSG_ERROR([$JAVA_ERROR_MESSAGE])
fi
AC_SUBST(JAR)


if test "x$GCC" = xyes; then
  JAW_CFLAGS="$JAW_CFLAGS $CFLAGS"
else
  AC_MSG_ERROR("You should compile with GCC")
fi
AC_SUBST(JAW_CFLAGS)

GNOME_COMPILE_WARNINGS([error])
case "$WARN_CFLAGS" in
    *-Werror*)
        WARN_CFLAGS="$WARN_CFLAGS -Wno-error=deprecated-declarations"
        ;;
esac
AM_CFLAGS="$AM_CFLAGS $WARN_CFLAGS"
AC_SUBST(AM_CFLAGS)

CHECK_JNI

AC_MSG_CHECKING([JDK version])
jversion=`$JAVA -version 2>&1 | head -n1 | cut -f2 -d'"' | cut -f1 -d_`
AC_MSG_RESULT($jversion)
j_major_version=0`echo $jversion | cut -f1 -d.`
j_minor_version=0`echo $jversion | cut -f2 -d.`
j_micro_version=0`echo $jversion | cut -f3 -d.`
min_major_version=0`echo $JAVA_REQUIRED | cut -f1 -d.`
min_minor_version=0`echo $JAVA_REQUIRED | cut -f2 -d.`
min_micro_version=0`echo $JAVA_REQUIRED | cut -f3 -d.`
if test "$j_major_version" -lt "$min_major_version"; then
  AC_MSG_ERROR([$JAVA_ERROR_MESSAGE])
elif test "$j_major_version" = "$min_major_version"; then
  if test "$j_minor_version" -lt "$min_minor_version"; then
    AC_MSG_ERROR([$JAVA_ERROR_MESSAGE])
  elif test "$j_minor_version" = "$min_minor_version"; then
    if test "$j_micro_version" -lt "$min_micro_version"; then
      AC_MSG_ERROR([$JAVA_ERROR_MESSAGE])
    fi
  fi
fi

XPROP_PATH=${PATH}:/usr/X11/bin
AC_PATH_PROG(XPROP,xprop,,${XPROP_PATH})
if test -z "$XPROP"; then
  AC_MSG_ERROR([No xprop found])
fi
AC_SUBST(XPROP)

AC_CONFIG_FILES([
    Makefile
    jni/Makefile
    jni/src/Makefile
    wrapper/Makefile
    wrapper/org/Makefile
    wrapper/org/GNOME/Makefile
    wrapper/org/GNOME/Accessibility/Makefile
])
AC_OUTPUT

echo "
Build configuration:

       Prefix:                                 ${prefix}
       Source code location:                   ${srcdir}
       Compiler:                               ${CC}
       JDK src:                                ${JDK_SRC}
       Host OS:                                ${host_os}
       JNI includes:                           ${JNI_INCLUDES}
       Warn flags:                             ${WARN_CFLAGS}
"
