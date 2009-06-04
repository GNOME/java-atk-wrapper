SUBDIRS=org
JARFILES=java-atk-wrapper.jar
CLASSES=$(wildcard $(srcdir)/org/GNOME/*/*.class)
JAVA_FILES=$(wildcard $(srcdir)/org/GNOME/*/*.java)

java_atk_wrapperdir = $(prefix)/share/jar
java_atk_wrapper_DATA = $(JARFILES)

EXTRA_DIST = $(properties_DATA)

propertiesdir = $(prefix)/share/jar
properties_DATA = accessibility.properties

$(JARFILES) :
	$(JAR) cf $(JARFILES) org/GNOME/Accessibility/*.class

all-local : $(DATA) $(JARFILES)

clean-local:
	rm $(JARFILES)
