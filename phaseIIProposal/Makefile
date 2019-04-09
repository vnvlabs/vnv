# LaTeX M akefile for dvi, ps, and pdf file creation.
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
SOURCES         = $(wildcard ./*.tex ./*/*.tex ./*.bib)
EPSFIGURES      = $(patsubst %.fig,%.ps,$(wildcard ./graphs/*.fig))

all : summary narrative company commercialization budget levelofeffort bio support complete 
summary: Summary.pdf
narrative: Narrative.pdf
company: Company.pdf
commercialization: Commercialization.pdf
budget: Budget.pdf
bio: Narayanan_cv.pdf Sabin_cv.pdf Billings_cv.pdf
complete: Complete.pdf
levelofeffort: LevelOfEffort.pdf

Commercialization.pdf:
	cp ./commercialization/cloudbench_commercialization.pdf ./Commercialization.pdf

SBC.pdf:
	cp ./company/SBC.pdf ./SBC.pdf

Billings_cv.pdf: 
	mv bio/billings_cv.pdf Billings_cv.pdf

Phase1Report.pdf:
	cp phase1finalreport_copy/phase1report.pdf ./Phase1Report.pdf

ORNL_LOC.pdf:
	cp ./budget/ORNL_LOC.pdf ./ORNL_LOC.pdf

EclipseLOS.pdf:
	cp letters/EclipseLOS.pdf EclipseLOS.pdf

Complete.pdf : Summary.pdf Narrative.pdf Sabin_cv.pdf Narayanan_cv.pdf Billings_cv.pdf Sabin_support.pdf Narayanan_support.pdf Budget.pdf LevelOfEffort.pdf Commercialization.pdf RNETHistory.pdf SBC.pdf Dataplan.pdf ORNL_LOC.pdf EclipseLOS.pdf Phase1Report.pdf
	pdftk Summary.pdf Narrative.pdf Sabin_cv.pdf Narayanan_cv.pdf Billings_cv.pdf Sabin_support.pdf Narayanan_support.pdf Budget.pdf ORNL_LOC.pdf LevelOfEffort.pdf Commercialization.pdf EclipseLOS.pdf RNETHistory.pdf SBC.pdf Dataplan.pdf cat output Complete.pdf


LevelOfEffort.pdf : 
	cp ./budget/level_of_effort.pdf ./LevelOfEffort.pdf

RNETHistory.pdf : Company.pdf
	pdftk Company.pdf ./company/RNET_Commercialization_History.pdf cat output RNETHistory.pdf
	rm Company.pdf

support : 
	pdftk ./bio/overall_support.pdf ./bio/narayanan_support.pdf cat output ./Narayanan_support.pdf
	pdftk ./bio/overall_support.pdf ./bio/sabin_support.pdf cat output ./Sabin_support.pdf

%.dvi : ${SOURCES} ${EPSFIGURES}
	./my_latex ${*}
	-bibtex ${*}
	./my_latex ${*}
	./my_latex ${*}
	@while ( grep "Rerun to get cross-references"                   \
		${*}.log .log> /dev/null ); do                   \
		echo '** Re-running LaTeX **';                          \
		./my_latex ${*};                                          \
	done

%.pdf : ${SOURCES} ${EPSFIGURES}
	./my_pdflatex ${*}
	-bibtex ${*}
	./my_pdflatex ${*}
	./my_pdflatex ${*}

#%.ps : %.dvi
#	dvips ${<} -o ${@}

%.rtf : %.dvi
	latex2rtf ${*}
	#echo "skipping rtf"

sched.bib:

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
