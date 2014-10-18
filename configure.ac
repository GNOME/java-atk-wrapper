AC_INIT([java-atk-wrapper],[0.30.7])

AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE([-Wall])

m4_pattern_allow([AM_PROG_AR])
AM_PROG_AR

# Before making a release, the LT_VERSION string should be modified.
# The string is of the form C:R:A.
# - If interfaces have been changed or added, but binary compatibility has
#   been preserved, change to C+1:0:A+1
# - If binary compatibility has been broken (eg removed or changed interfaces)
#   change to C+1:0:0
# - If the interface is the same as the previous version, change to C:R+1:A

LIB_ATK_WRAPPER_LT_VERSION=0:15:0
AC_SUBST(LIB_ATK_WRAPPER_LT_VERSION)

# jni

AC_CONFIG_HEADERS([config.h])

AC_PROG_CC
AC_PROG_INSTALL
AC_ISC_POSIX

AM_PROG_LIBTOOL

AC_CONFIG_MACRO_DIR([m4])

m4_define([req_ver_atk], [1.17.0])

PKG_CHECK_MODULES(JAW, [
	atk             >= req_ver_atk
	glib-2.0
	gthread-2.0
	gmodule-2.0
	gdk-2.0
	gconf-2.0
])

ATK_BRIDGE_LIB_PATH="`pkg-config --variable=libdir gtk+-2.0`/gtk-2.0"
AC_SUBST(ATK_BRIDGE_LIB_PATH)

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

JAVA_REQUIRED=1.5
JAVA_ERROR_MESSAGE="Java $JAVA_REQUIRED or later is required to build java-access-bridge"

AC_ARG_VAR([JRE_SRC],[Java Runtime Environment location])
AC_ARG_VAR([JDK_SRC],[JDK installation path])

if test -z "$JRE_SRC"; then
  JAVA_PATH=${PATH}
else
  JAVA_PATH=${JRE_SRC}:${JRE_SRC}/jre/bin:${JRE_SRC}/bin:${JRE_SRC}/../bin:${PATH}
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

AC_ARG_WITH([jdk_auto_detect],
	    [AS_HELP_STRING([--with-jdk-auto-detect],
			    [Auto-detection of jdk location])],
	    [],
	    [with_jdk_auto_detect=yes])

if test "x$with_jdk_auto_detect" = "xyes"; then
	CUR_DIR=`pwd`
	SRC_DIR=`dirname $0`
	SRC_DIR=`cd $SRC_DIR;pwd;cd $CUR_DIR`
	$JAVAC $JAVACFLAGS $SRC_DIR/JavaInfo.java
	JDK_SRC="`CLASSPATH=$SRC_DIR $JAVA JavaInfo`"
else
	if test -z "$JDK_SRC"; then
		AC_MSG_ERROR("You should specify JDK_SRC")
	fi
fi
AC_SUBST(JDK_SRC)

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

AC_OUTPUT([
    Makefile
    jni/Makefile
    jni/src/Makefile
    wrapper/Makefile
    wrapper/org/Makefile
    wrapper/org/GNOME/Makefile
    wrapper/org/GNOME/Accessibility/Makefile
])