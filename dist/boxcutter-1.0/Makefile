# Makefile for boxcutter

VERSION = 1.0
FILES = \
	boxcutter.cpp \
	boxcutter.exe \
	Makefile \
	README.txt \
	COPYING.LESSER \
	LICENSE


boxcutter: boxcutter.cpp
	g++ boxcutter.cpp -o boxcutter -mwindows \
		-lcomctl32 -lgdi32

distrib:
	mkdir -p dist
	rm -rf dist/boxcutter-$(VERSION)
	mkdir -p dist/boxcutter-$(VERSION)
	cp $(FILES) dist/boxcutter-$(VERSION)
	tar zcvf dist/boxcutter-$(VERSION).tar.gz dist/boxcutter-$(VERSION)

clean:
	rm -f boxcutter.exe