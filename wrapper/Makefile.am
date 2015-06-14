SUBDIRS=org
JARFILES=java-atk-wrapper.jar
ALL_CLASSES=org/GNOME/Accessibility/*.class
DEP_CLASSES=$(wildcard $(ALL_CLASSES))

if USER
java_atk_wrapperdir = $(JDK_SRC)/jre/lib/ext
else
java_atk_wrapperdir = $(HOME)/.local$(JDK_SRC)/jre/lib/ext
endif
java_atk_wrapper_DATA = $(JARFILES)

if USER
propertiesdir = $(JDK_SRC)/jre/lib
else
propertiesdir = $(HOME)/.local$(JAVA_HOME)/lib
endif

properties_DATA = accessibility.properties
EXTRA_DIST = $(properties_DATA)

$(JARFILES) : $(DEP_CLASSES)
	$(JAR) cf $(JARFILES) $(ALL_CLASSES)

all-local : $(DATA) $(JARFILES)

clean-local:
	rm -f $(JARFILES)
