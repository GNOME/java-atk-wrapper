SUBDIRS=org
JARFILES=java-atk-wrapper.jar
ALL_CLASSES=org/GNOME/Accessibility/*.class
DEP_CLASSES=$(wildcard $(ALL_CLASSES))

java_atk_wrapperdir = $(prefix)/share/jar
java_atk_wrapper_DATA = $(JARFILES)

EXTRA_DIST = $(properties_DATA)

propertiesdir = $(prefix)/share/jar
properties_DATA = accessibility.properties

$(JARFILES) : $(DEP_CLASSES)
	$(JAR) cf $(JARFILES) $(ALL_CLASSES)

all-local : $(DATA) $(JARFILES)

clean-local:
	rm -f $(JARFILES)
