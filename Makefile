# Makefile for boxcutter

VERSION = 1.5

FILES = boxcutter.cpp \
	boxcutter-fs.cpp \
	boxcutter.exe \
	boxcutter-fs.exe \
	Makefile \
	README.txt \
	COPYING.LESSER \
	LICENSE \
	gdi
CC=c:/mingw/bin/g++

WWW = /var/www/dev/rasm/boxcutter/download

CFLAGS=-mwindows -lcomctl32 -lgdi32 -I/usr/include/wine/msvcrt -Lgdi -lgdiplus

all: boxcutter.exe boxcutter-fs.exe

boxcutter.exe: boxcutter.cpp png.cpp
	$(CC) boxcutter.cpp -o boxcutter $(CFLAGS)

boxcutter-fs.exe: boxcutter-fs.cpp
	$(CC) boxcutter-fs.cpp -o boxcutter-fs $(CFLAGS)


pkg: $(FILES)
	mkdir -p dist
	rm -rf dist/boxcutter-$(VERSION)
	mkdir -p dist/boxcutter-$(VERSION)
	cp -r $(FILES) dist/boxcutter-$(VERSION)
	cd dist && zip -r boxcutter-$(VERSION).zip boxcutter-$(VERSION)

upload:
	cp -r dist/boxcutter-$(VERSION).zip \
              dist/boxcutter-$(VERSION) $(WWW)

clean:
	rm -f boxcutter.exe boxcutter-fs.exe



