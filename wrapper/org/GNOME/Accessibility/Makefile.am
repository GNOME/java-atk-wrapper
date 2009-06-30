JAVA_SRC=$(wildcard $(srcdir)/*.java)
CLASSES=$(patsubst %.java,%.class,$(JAVA_SRC))
BUILD_CLASSES=$(patsubst $(srcdir)%,$(builddir)%,$(CLASSES))

EXTRA_DIST=$(JAVA_SRC)

all-local: $(BUILD_CLASSES)

%.class : %.java
	CLASSPATH=$(top_srcdir)/wrapper $(JAVAC) -d $(top_builddir)/wrapper $<

clean-local:
	$(RM) *.class


