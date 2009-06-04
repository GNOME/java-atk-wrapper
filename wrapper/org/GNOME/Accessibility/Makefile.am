JAVA_SRC=$(wildcard $(srcdir)/*.java)
CLASSES=$(patsubst $(srcdir)/%.java,%.class,$(JAVA_SRC))

EXTRA_DIST=$(JAVA_SRC)

all-local: $(CLASSES)

%.class : $(srcdir)/%.java
	CLASSPATH=$(top_srcdir)/wrapper $(JAVAC) -d $(top_builddir)/wrapper $<

clean-local:
	$(RM) *.class


