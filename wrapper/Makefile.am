SUBDIRS=org
JARFILES=java-atk-wrapper.jar
DEP_CLASSES=$(wildcard org/GNOME/Accessibility/*.class)

java_atk_wrapperdir = $(JAVA_HOME)/jre/lib/ext
java_atk_wrapper_DATA = $(JARFILES)

EXTRA_DIST = $(properties_DATA)

propertiesdir = $(JAVA_HOME)/jre/lib
properties_DATA = accessibility.properties

$(JARFILES) : $(DEP_CLASSES)
	$(JAR) cf $(JARFILES) org/GNOME/Accessibility/*.class

all-local : $(DATA) $(JARFILES)

clean-local:
	rm -f $(JARFILES)
