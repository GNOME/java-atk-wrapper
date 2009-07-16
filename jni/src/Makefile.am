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
			    jawtable.c

noinst_HEADERS = jawimpl.h \
		  jawobject.h \
		  jawhyperlink.h \
		  jawtoplevel.h \
		  jawutil.h

libatk_wrapper_la_CPPFLAGS = $(JNI_INCLUDES)\
			     $(JAW_CFLAGS)\
			     -DATK_BRIDGE_LIB_NAME=\"$(ATK_BRIDGE_LIB_NAME)\"

libatk_wrapper_la_LDFLAGS = -version-info $(LIB_ATK_WRAPPER_LT_VERSION)

libatk_wrapper_la_LIBADD = $(JAW_LIBS)

