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
bio: ONeill_cv.pdf Sabin_cv.pdf Nguyen_cv.pdf Watson_cv.pdf
complete: Complete.pdf
levelofeffort: LevelOfEffort.pdf

Commercialization.pdf:
	cp ./commercialization/commercialization.pdf ./Commercialization.pdf

SBA.pdf:
	cp ./company/SBA.pdf ./SBA.pdf

Phase1Report.pdf:
	cp ../phaseIReport/FinalReport.pdf ./Phase1Report.pdf

ORNL_LOC.pdf:
	pdftk ./budget/ORNL_LOC.pdf ./budget/ORNL_Budget.pdf cat output ./ORNL_LOC.pdf

Watson_cv.pdf:
	cp ./bio/GWatsonCV2018.pdf ./Watson_cv.pdf

Complete.pdf : Summary.pdf Narrative.pdf Oneill_cv.pdf Sabin_cv.pdf Nguyen_cv.pdf Watson_cv.pdf Oneill_support.pdf Sabin_support.pdf Nguyen_support.pdf Budget.pdf LevelOfEffort.pdf Commercialization.pdf RNETHistory.pdf SBA.pdf Dataplan.pdf ORNL_LOC.pdf Phase1Report.pdf
	pdftk Summary.pdf Narrative.pdf Oneill_cv.pdf Sabin_cv.pdf Nguyen_cv.pdf Watson_cv.pdf Oneill_support.pdf Sabin_support.pdf Nguyen_support.pdf Budget.pdf LevelOfEffort.pdf Commercialization.pdf RNETHistory.pdf SBA.pdf Dataplan.pdf ORNL_LOC.pdf Phase1Report.pdf cat output Complete.pdf


LevelOfEffort.pdf : 
	cp ./budget/level_of_effort.pdf ./LevelOfEffort.pdf

RNETHistory.pdf : Company.pdf
	pdftk Company.pdf ./company/RNET_Commercialization_History.pdf cat output RNETHistory.pdf
	rm Company.pdf

support : 
	pdftk ./bio/overall_support.pdf ./bio/oneill_support.pdf cat output ./Oneill_support.pdf
	pdftk ./bio/overall_support.pdf ./bio/sabin_support.pdf cat output ./Sabin_support.pdf
	pdftk ./bio/overall_support.pdf ./bio/nguyen_support.pdf cat output ./Nguyen_support.pdf

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
	rm -f ./*.toc
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
