SUBDIRS=org
JARFILES=java-atk-wrapper.jar
ALL_CLASSES=org/GNOME/Accessibility/*.class
DEP_CLASSES=$(wildcard $(ALL_CLASSES))

if USER
java_atk_wrapperdir = $(JAVA_HOME)/lib/ext
else
java_atk_wrapperdir = $(prefix)/jre/lib/ext
endif

java_atk_wrapper_DATA = $(JARFILES)

EXTRA_DIST = $(properties_DATA)

if USER
propertiesdir = $(JAVA_HOME)/lib
else
propertiesdir = $(prefix)/jre/lib
endif

properties_DATA = accessibility.properties

$(JARFILES) : $(DEP_CLASSES)
	$(JAR) cf $(JARFILES) $(ALL_CLASSES)

all-local : $(DATA) $(JARFILES)

clean-local:
	rm -f $(JARFILES)
