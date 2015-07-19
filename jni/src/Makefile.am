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
                            jawtablecell.c

noinst_HEADERS = AtkWrapper.h \
                 jawimpl.h \
                 jawobject.h \
                 jawhyperlink.h \
                 jawtoplevel.h \
                 jawutil.h

libatk_wrapper_la_CFLAGS = $(JNI_INCLUDES)\
                           $(JAW_CFLAGS) \
                           $(AM_CFLAGS) \
                           $(PLATFORM_CFLAGS)

libatk_wrapper_la_CPPFLAGS = $(JNI_INCLUDES)\
                             $(JAW_CFLAGS) \
                             $(PLATFORM_CFLAGS)

libatk_wrapper_la_LDFLAGS = $(LIB_ATK_WRAPPER_LD_FLAGS) \
                            $(LT_VERSION_INFO) \
                            $(PLATFORM_LDFLAGS) \
                            $(AM_LDFLAGS)

libatk_wrapper_la_LIBADD = $(JAW_LIBS)

