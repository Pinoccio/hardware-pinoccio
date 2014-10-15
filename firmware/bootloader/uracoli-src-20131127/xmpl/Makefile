PROJECTS = $(wildcard *.mk)
BUILDS = $(addprefix BUILD_, $(PROJECTS))
CLEANS = $(addprefix CLEAN_, $(PROJECTS))
all: $(BUILDS)
clean: $(CLEANS)
BUILD_%: %
	make -f $< all
CLEAN_%:  %
	make -f $< clean