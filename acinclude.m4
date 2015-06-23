AC_DEFUN([CHECK_JNI],[

dnl Check to see what platform and set jni include path
AC_CANONICAL_HOST
AC_MSG_CHECKING([platform to setup platform specific variables])
platform_win32="no"
case "$host" in
  *-*-msdos* | *-*-go32* | *-*-mingw32* | *-*-windows*)
    if test "$gcj_compile" = "yes"; then
      JNI_INCLUDES=
    else
      JNI_INCLUDES="-I$JDK_SRC/include -I$JDK_SRC/include/win32"
    fi
    platform_win32="yes"
    PLATFORM_CFLAGS="-mms-bitfields"
    PLATFORM_LDFLAGS="-Wl,--kill-at"
    PLATFORM_CLASSPATH_SEPARATOR=";"
    SOPREFIX="lib"
    ;;
  *-*-cygwin*)
    if test "$gcj_compile" = "yes"; then
      JNI_INCLUDES=
    else
      JNI_INCLUDES="-I$JDK_SRC/include -I$JDK_SRC/include/win32"
    fi
    platform_win32="yes"
    PLATFORM_CFLAGS=
    PLATFORM_LDFLAGS=
    PLATFORM_CLASSPATH_SEPARATOR=":"
    SOPREFIX="cyg"
    ;;
  *-*-linux*)
    if test "$gcj_compile" = "yes"; then
      JNI_INCLUDES=
    else
      JNI_INCLUDES="-I$JDK_SRC/include -I$JDK_SRC/include/linux"
    fi
    PLATFORM_CFLAGS=
    PLATFORM_LDFLAGS="-rpath $libdir"
    PLATFORM_CLASSPATH_SEPARATOR=":"
    SOPREFIX=
    ;;
  *-*-solaris*)
    if test "$gcj_compile" = "yes"; then
      JNI_INCLUDES=
    else
      JNI_INCLUDES="-I$JDK_SRC/include -I$JDK_SRC/include/solaris"
    fi
    PLATFORM_CFLAGS=
    PLATFORM_LDFLAGS=
    PLATFORM_CLASSPATH_SEPARATOR=":"
    SOPREFIX=
    ;;
  *-*-darwin*)
    if test "$gcj_compile" = "yes"; then
      JNI_INCLUDES=
    else
      JNI_INCLUDES="-I$JDK_SRC/include -I$JDK_SRC/include/darwin"
    fi
    PLATFORM_CFLAGS=
    PLATFORM_LDFLAGS=
    PLATFORM_CLASSPATH_SEPARATOR=":"
    SOPREFIX=
    ;;
  *)
    if test "$gcj_compile" = "yes"; then
      JNI_INCLUDES=
    else
      JNI_INCLUDES="-I$JDK_SRC/include -I$JDK_SRC/include/$host_os"
    fi
    PLATFORM_CFLAGS=
    PLATFORM_LDFLAGS=
    PLATFORM_CLASSPATH_SEPARATOR=":"
    SOPREFIX=
    ;;
esac
AC_MSG_RESULT([$host_os])
AC_SUBST(JNI_INCLUDES)
AC_SUBST(PLATFORM_CFLAGS)
AC_SUBST(PLATFORM_LDFLAGS)
AC_SUBST(PLATFORM_CLASSPATH_SEPARATOR)

])
