lib_LTLIBRARIES = libatk-wrapper.la

libatk_wrapper_la_SOURCES = AtkWrapper.c \
                            jawutil.c \
                            jawtoplevel.c \
                            jawobject.c \
                            jawimpl.c \
                            jawaction.c \
                            jawcomponent.c \
                            jawtext.c \
                            jaweditabletext.c \
                            jawhyperlink.c \
                            jawhypertext.c \
                            jawimage.c \
                            jawselection.c \
                            jawvalue.c \
                            jawtable.c \
                            jawwindow.c

noinst_HEADERS = jawimpl.h \
                 jawobject.h \
                 jawhyperlink.h \
                 jawtoplevel.h \
                 jawutil.h \
                 jawwindow.h

libatk_wrapper_la_CFLAGS = $(JAW_CFLAGS) \
                           $(JNI_INCLUDES)

libatk_wrapper_la_CPPFLAGS = $(JAW_CFLAGS) \
                             $(JNI_INCLUDES)

libatk_wrapper_la_LDFLAGS = $(LIB_ATK_WRAPPER_LD_FLAGS) \
                            $(LT_VERSION_INFO) \
                            -rpath $(libdir) \
                            $(AM_LDFLAGS)

libatk_wrapper_la_LIBADD = $(JAW_LIBS)

