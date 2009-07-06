JAVA_SRC=$(wildcard $(srcdir)/*.java)
CLASSES=$(patsubst %.java,%.class,$(JAVA_SRC))
BUILD_CLASSES=$(patsubst $(srcdir)%,$(builddir)%,$(CLASSES)) $(builddir)/AtkWrapper.class

EXTRA_DIST=$(JAVA_SRC) AtkWrapper.java.in

all-local: $(BUILD_CLASSES)

AtkWrapper.java : AtkWrapper.java.in
	sed -e "s;\@XPROP\@;${XPROP};g" \
		< $< >$@

%.class : %.java
	CLASSPATH=$(top_srcdir)/wrapper $(JAVAC) -d $(top_builddir)/wrapper $<

clean-local:
	$(RM) *.class
	$(RM) AtkWrapper.java

