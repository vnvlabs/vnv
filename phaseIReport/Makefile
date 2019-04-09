# LaTeX Makefile for dvi, ps, and pdf file creation.
# By Jeffrey Humpherys
# Written April 05, 2004
# Revised January 13, 2005
# Thanks Bjorn and Boris

# Edited heavily by Gerald Sabin April 02, 2005

#
# Usage:
# make          # make dvi, ps, and pdf
# make dvi      # make dvi
# make ps       # make ps (and dvi)
# make pdf      # make pdf
#

MAIN            = main
SOURCES         = $(wildcard ./*.tex ./*/*.tex ./proposal.bib bib/*.bib)
EPSFIGURES      = $(patsubst %.fig,%.ps,$(wildcard ./graphs/*.fig))

all : FinalReport.pdf

%.pdf : ${SOURCES} ${EPSFIGURES}
	./my_pdflatex ${*}
	-bibtex ${*}
	./my_pdflatex ${*}
	./my_pdflatex ${*}

clean:
	rm -f ./*.log
	rm -f ./*.aux
	rm -f ./*.out
	rm -f ./*.bbl
	rm -f ./*.blg
	rm -f ./*~
	rm -f ./*.bak
	rm -f ./\#*\#
	rm -f ./*/*~
	rm -f ./*/*.bak
	rm -f ./*/\#*\#
	rm -f ./*/*/*~
	rm -f ./*/*/*.bak
	rm -f ./*/*/\#*\#

veryclean: clean
	rm -f *.ps
	rm -f *.dvi
	rm -f *.rtf

pdfclean: veryclean
	rm *.pdf
