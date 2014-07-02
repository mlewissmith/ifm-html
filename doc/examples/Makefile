# Makefile for ruins demo.

all: ruins-items.txt ruins-tasks.txt ruins-rec.txt

%.scr: ruins.z5 %.rec
	rec2scr -o $@ -r $*.rec ruins.z5

%-rec.txt: %.ifm
	ifm -t -f rec -o $@ $<

%-items.txt: %.ifm
	ifm -i -o $@ $<

%-tasks.txt: %.ifm
	ifm -t -o $@ $<
