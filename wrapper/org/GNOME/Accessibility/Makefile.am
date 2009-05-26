JAVA_SRC=$(wildcard *.java)
CLASSES=$(patsubst %.java,%.class,$(JAVA_SRC))

EXTRA_DIST=$(JAVA_SRC)

all-local: $(CLASSES)

%.class : %.java
	CLASSPATH=$(top_srcdir)/wrapper $(JAVAC) $<

clean-local:
	$(RM) *.class


