SOURCES=$(wildcard *.tex)
IMAGES=$(SOURCES:.tex=.pdf)

all: $(IMAGES)

$(IMAGES):%.pdf: %.tex
	echo $(IMAGES)
	pdflatex $<
	rm *.aux *.log
