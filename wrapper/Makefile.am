SUBDIRS=org

if MODULAR_JAR
  SUBDIRS += META-INF
endif

JARFILES=@JAW_JAR_NAME@
ALL_CLASSES=org/GNOME/Accessibility/*.class
DEP_CLASSES=$(wildcard $(ALL_CLASSES))

java_atk_wrapperdir = $(JARDIR)
propertiesdir = $(PROPERTIESDIR)

java_atk_wrapper_DATA = $(JARFILES)
properties_DATA = accessibility.properties
EXTRA_DIST = $(properties_DATA) \
	     manifest.txt module-info.java

if MODULAR_JAR

module-info.class : $(srcdir)/module-info.java
	$(JAVAC) -cp $(top_builddir)/wrapper -sourcepath $(top_srcdir)/wrapper:$(top_builddir)/wrapper $(JAVACFLAGS) -d $(top_builddir)/wrapper $<

$(JARFILES) : $(DEP_CLASSES) module-info.class
	$(JAR) cfm $(JARFILES) $(srcdir)/manifest.txt org/GNOME/Accessibility/*.class module-info.class -C $(srcdir) META-INF/services/javax.accessibility.AccessibilityProvider

else

$(JARFILES) : $(DEP_CLASSES)
	$(JAR) cfm $(JARFILES) $(srcdir)/manifest.txt org/GNOME/Accessibility/*.class

endif

install-data-local : all-local
	$(mkinstalldirs) $(DESTDIR)$(propertiesdir) $(DESTDIR)$(java_atk_wrapperdir)
	$(INSTALL_DATA) $(srcdir)/$(properties_DATA) $(DESTDIR)$(propertiesdir)/
	$(INSTALL_DATA) $(java_atk_wrapper_DATA) $(DESTDIR)$(java_atk_wrapperdir)/

all-local : $(DATA) $(JARFILES)

clean-local:
	$(RM) *.class
	rm -f $(JARFILES)
