.PHONY: clean paper

all: paper

paper:  
	pdflatex dac20_main.tex
	bibtex dac20_main.aux
	pdflatex dac20_main.tex
	pdflatex dac20_main.tex

clean:
	@rm -f *.bbl *.blg *.aux *.fls *.log *.pdf *.synctex.gz
