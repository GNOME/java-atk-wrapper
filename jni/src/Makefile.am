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
			     -DATK_BRIDGE_LIB_PATH=\"$(ATK_BRIDGE_LIB_PATH)\"

libatk_wrapper_la_LDFLAGS = $(LIB_ATK_WRAPPER_LD_FLAGS)

libatk_wrapper_la_LIBADD = $(JAW_LIBS)

