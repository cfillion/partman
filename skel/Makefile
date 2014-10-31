all: lilypond midi
lilypond:
	lilypond -l WARNING --include partman *.ly 2>&1 | ccze -A
midi: lilypond
	timidity --quiet=1 -Oj *.midi 2>/dev/null
clean:
	rm *.midi *.pdf
