SUBDIRS=org
JARFILES=java-atk-wrapper.jar
ALL_CLASSES=org/GNOME/Accessibility/*.class
DEP_CLASSES=$(ALL_CLASSES)

if USER
java_atk_wrapperdir = $(JAVA_HOME)/lib/ext
propertiesdir = $(JAVA_HOME)/lib
else
java_atk_wrapperdir = $(HOME)/.local/share/java-atk-wrapper$(JAVA_HOME)/lib/ext
propertiesdir = $(HOME)/.local/share/java-atk-wrapper$(JAVA_HOME)/lib
endif

java_atk_wrapper_DATA = $(JARFILES)
properties_DATA = accessibility.properties
EXTRA_DIST = $(properties_DATA)
dist-hook:
srcdir_cnt=`echo $(srcdir) | wc -c`; \
for file in $(ALL_CLASSES); do \
f=`echo $$file | cut -c$$srcdir_cnt-`; \
fdir=`dirname $$f`; \
if test ! -d $(distdir)/$$fdir; then \
echo "$(makeinstalldirs) $(distdir)/$$fdir"; \
$(mkinstalldirs) $(distdir)/$$fdir; \
fi; \
echo "cp -p $$file $(distdir)/$$f"; \
cp -p $$file $(distdir)/$$f; \
done

$(JARFILES) : $(DEP_CLASSES)
	$(JAR) cfm $(JARFILES) manifest.txt org/GNOME/Accessibility/*.class

all-local : $(DATA) $(JARFILES)

clean-local:
	rm -f $(JARFILES)
