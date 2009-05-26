SUBDIRS=org
JARFILES=java-atk-wrapper.jar
CLASSES=$(wildcard org/GNOME/*/*.class)
JAVA_FILES=$(wildcard org/GNOME/*/*.java)
DEP_CLASSES=$(JAVA_FILES:.java=.class)

java_atk_wrapperdir = $(prefix)/java/jre/lib/ext
java_atk_wrapper_DATA = $(JARFILES)

EXTRA_DIST = $(properties_DATA)

propertiesdir = $(prefix)/java/jre/lib
properties_DATA = accessibility.properties

jar-stamp : jar-stamp
	echo

$(JARFILES) : jar-stamp $(JAVA_FILES) $(DEP_CLASSES)
	touch jar-stamp
	$(JAR) cf $(JARFILES) org/GNOME/Accessibility/*.class

$(DEP_CLASSES) :
	@(cd `dirname $@`; $(MAKE) )

all-local : $(DATA) $(JARFILES)

