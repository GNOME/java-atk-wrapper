SUBDIRS=org
JARFILES=@JAW_JAR_NAME@
ALL_CLASSES=org/GNOME/Accessibility/*.class
DEP_CLASSES=$(wildcard $(ALL_CLASSES))

java_atk_wrapperdir = $(JARDIR)
propertiesdir = $(PROPERTIESDIR)

java_atk_wrapper_DATA = $(JARFILES)
properties_DATA = accessibility.properties
EXTRA_DIST = $(properties_DATA) \
	     manifest.txt

$(JARFILES) : $(DEP_CLASSES)
	$(JAR) cfm $(JARFILES) $(srcdir)/manifest.txt org/GNOME/Accessibility/*.class

install-data-local : all-local
	$(mkinstalldirs) $(DESTDIR)$(propertiesdir) $(DESTDIR)$(java_atk_wrapperdir)
	$(INSTALL_DATA) $(srcdir)/$(properties_DATA) $(DESTDIR)$(propertiesdir)/
	$(INSTALL_DATA) $(java_atk_wrapper_DATA) $(DESTDIR)$(java_atk_wrapperdir)/

all-local : $(DATA) $(JARFILES)

clean-local:
	$(RM) *.class
	rm -f $(JARFILES)
