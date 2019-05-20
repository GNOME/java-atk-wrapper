JAVA_SRC=$(filter-out $(srcdir)/AtkWrapper.java, $(filter-out $(srcdir)/AtkProvider.java, $(wildcard $(srcdir)/*.java)))

if MODULAR_JAR
  JAVA_SRC += $(srcdir)/AtkProvider.java
endif

CLASSES=$(patsubst %.java,%.class,$(JAVA_SRC))
BUILD_CLASSES=$(patsubst $(srcdir)%,$(builddir)%,$(CLASSES)) $(builddir)/AtkWrapper.class

EXTRA_DIST=$(JAVA_SRC) AtkWrapper.java.in $(srcdir)/AtkProvider.java

all-local: $(BUILD_CLASSES)

%.class : %.java AtkWrapper.java
	$(JAVAC) -cp $(top_builddir)/wrapper -sourcepath $(top_srcdir)/wrapper:$(top_builddir)/wrapper $(JAVACFLAGS) -d $(top_builddir)/wrapper $<

clean-local:
	$(RM) *.class
	$(RM) AtkWrapper.java

