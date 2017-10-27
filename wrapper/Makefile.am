SUBDIRS=org
JARFILES=java-atk-wrapper.jar
ALL_CLASSES=org/GNOME/Accessibility/*.class
DEP_CLASSES=$(wildcard $(ALL_CLASSES))

java_atk_wrapperdir = $(JARDIR)
propertiesdir = $(PROPERTIESDIR)

java_atk_wrapper_DATA = $(JARFILES)
properties_DATA = accessibility.properties
EXTRA_DIST = $(properties_DATA) \
	     manifest.txt

$(JARFILES) : $(DEP_CLASSES)
	$(JAR) cfm $(JARFILES) manifest.txt org/GNOME/Accessibility/*.class

all-local : $(DATA) $(JARFILES)

clean-local:
	rm -f $(JARFILES)
