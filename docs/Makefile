# versatile Tex Makefile
TEX			:= pdflatex
TEXFLAGS	:=

# sources
SOURCES		:= $(shell find -name "*.tex")
# pdfs (tex'ed from SOURCES)
ALL			:= $(subst .tex,.pdf,$(SOURCES))

all: $(ALL)

# tex sources
$(ALL) : %.pdf : %.tex
	$(TEX) -draftmode $(TEXFLAGS) $<
	$(TEX) $(TEXFLAGS) $<

clean:
	-rm -f *.log *.aux *.toc *~ $(ALL)

debug:
	@echo $(SOURCES)

