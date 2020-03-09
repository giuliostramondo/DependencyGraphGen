.PHONY: clean paper

all: paper

paper:
	pdflatex dsd20_main.tex
	bibtex dsd20_main.aux
	pdflatex dsd20_main.tex
	pdflatex dsd20_main.tex

clean:
	@rm -f *.bbl *.blg *.aux *.fls *.log *.pdf *.synctex.gz
