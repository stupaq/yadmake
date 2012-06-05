# versatile Tex Makefile
TEX			:= pdflatex
TEXFLAGS	:=

# sources
SOURCES		:= $(shell find -name "*.tex")
# pdfs (tex'ed from SOURCES)
ALL			:= $(subst .tex,.pdf,$(SOURCES))
# package
PACKAGE		:= yadmake-docs

all: $(ALL)

# tex sources
$(ALL) : %.pdf : %.tex
	$(TEX) -draftmode $(TEXFLAGS) $<
	$(TEX) $(TEXFLAGS) $<

clean:
	rm -f *.log *.aux *.toc *~ $(ALL) 
	rm -rf $(PACKAGE).tgz $(PACKAGE)/

debug:
	@echo $(SOURCES)

package: all
	mkdir $(PACKAGE)
	mv $(ALL) $(PACKAGE)
	tar czf $(PACKAGE).tgz $(PACKAGE)/

