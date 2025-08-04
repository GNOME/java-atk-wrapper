AC_PREREQ(2.63)
AC_INIT([java-atk-wrapper],[0.42.1],[https://gitlab.gnome.org/GNOME/java-atk-wrapper/issues/new])
: ${CXXFLAGS=""}

AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE([1.11 -Wall check-news])

AM_SILENT_RULES([yes])
m4_pattern_allow([AM_PROG_AR])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

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
  atk >= 2.36.0
  atk-bridge-2.0 >= 2.33.1
  atspi-2 >= 2.14.0
  dbus-1
  glib-2.0 >= 2.32.0
  gthread-2.0
  gobject-2.0
])

LIB_ATK_WRAPPER_LD_FLAGS="-avoid-version -no-undefined"
AC_SUBST(LIB_ATK_WRAPPER_LD_FLAGS)

AC_ARG_ENABLE([modular-jar],
            [AS_HELP_STRING([--enable-modular-jar],
            [Build modular java-atk-wrapper jar])],
            [enable_modular_jar="$enableval"],
            [enable_modular_jar=no])
AS_IF([test "x$enable_modular_jar" = "xyes"], [
  JAVA_REQUIRED=9.0.0
], [
  JAVA_REQUIRED=1.7.0
])
JAVA_ERROR_MESSAGE="Java $JAVA_REQUIRED or later is required to build java-access-bridge"
AM_CONDITIONAL([MODULAR_JAR], [test "x$enable_modular_jar" = "xyes"])

# java wrapper

AM_CONDITIONAL(USER, test `whoami` = "root")

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
  `javac -nowarn $JAVACFLAGS ${srcdir}/JavaInfo.java -d .`
  JDK_SRC=`java JavaInfo`
  JAVA_HOME="${JDK_SRC}/jre"
  if test ! -d ${JAVA_HOME}; then
    JAVA_HOME="${JDK_SRC}"
  fi
else
  if test -z "$JDK_SRC"; then
    AC_MSG_ERROR("You should specify JDK_SRC")
  else
    if test -z "$JAVA_HOME"; then
      JAVA_HOME="${JDK_SRC}/jre"
      if test ! -d ${JAVA_HOME}; then
        JAVA_HOME="${JDK_SRC}"
      fi
    fi
  fi
fi

AC_SUBST(JDK_SRC)
AC_SUBST(JAVA_HOME)

if test -z "$JAVA_HOME"; then
  JAVA_PATH=${PATH}
else
  JAVA_PATH=${JDK_SRC}:${JDK_SRC}/bin:${PATH}
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

AX_IS_RELEASE(dash-version)
AX_COMPILER_FLAGS()
case "$WARN_CFLAGS" in
    *-Werror*)
        WARN_CFLAGS="$WARN_CFLAGS -Wno-error=deprecated-declarations"
        WARN_CFLAGS="$WARN_CFLAGS -Wno-error=strict-prototypes"
        WARN_CFLAGS="$WARN_CFLAGS -Wno-error=declaration-after-statement"
        ;;
esac
AM_CFLAGS="$AM_CFLAGS $WARN_CFLAGS -Wno-declaration-after-statement"
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

AC_ARG_WITH(jardir, AS_HELP_STRING([--with-jardir=[dirname]],[Directory for installing the java-atk-wrapper.jar file]))
if test -z "$with_jardir" ; then
    if test -d ${JAVA_HOME}/lib/ext ; then
        if test "x$(basename ${JAVA_HOME})" = xjre; then
            JARDIR=${libdir}/jvm/$(basename ${JDK_SRC})/jre/lib/ext
        else
            JARDIR=${libdir}/jvm/$(basename ${JAVA_HOME})/lib/ext
        fi
    else
        JARDIR=${libdir}/jvm/$(basename ${JAVA_HOME})/lib
    fi
else
    JARDIR=$with_jardir
fi
AC_SUBST(JARDIR)

AC_ARG_WITH(propertiesdir, AS_HELP_STRING([--with-propertiesdir=[dirname]],[Directory for installing the accessibility.properties file]))
if test -z "$with_propertiesdir" ; then
    if test -d ${JAVA_HOME}/conf ; then
        PROPERTIESDIR=${libdir}/jvm/$(basename ${JAVA_HOME})/conf
    else
        if test "x$(basename ${JAVA_HOME})" = xjre; then
            PROPERTIESDIR=${libdir}/jvm/$(basename ${JDK_SRC})/jre/lib
        else
            PROPERTIESDIR=${libdir}/jvm/$(basename ${JAVA_HOME})/lib
        fi
    fi
else
    PROPERTIESDIR=$with_propertiesdir
fi
AC_SUBST(PROPERTIESDIR)

JAW_JAR_NAME=java-atk-wrapper.jar
AC_SUBST(JAW_JAR_NAME)

AC_CONFIG_FILES([
    Makefile
    jni/Makefile
    jni/src/Makefile
    wrapper/Makefile
    wrapper/org/Makefile
    wrapper/org/GNOME/Makefile
    wrapper/org/GNOME/Accessibility/AtkWrapper.java
    wrapper/org/GNOME/Accessibility/Makefile
    wrapper/META-INF/Makefile
    wrapper/META-INF/services/Makefile
])
AC_OUTPUT

echo "
Build configuration:

       Prefix:                                 ${prefix}
       Source code location:                   ${srcdir}
       Modular jar:                            ${enable_modular_jar}
       Compiler:                               ${CC}
       JDK src:                                ${JDK_SRC}
       JAVA home:                              ${JAVA_HOME}
       Host OS:                                ${host_os}
       JNI includes:                           ${JNI_INCLUDES}
       Warn flags:                             ${WARN_CFLAGS}
"
