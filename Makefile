# Makefile for boxcutter

VERSION = 1.3
FILES = boxcutter.cpp \
	boxcutter-fs.cpp \
	boxcutter.exe \
	boxcutter-fs.exe \
	Makefile \
	README.txt \
	COPYING.LESSER \
	LICENSE
CC=c:/mingw/bin/g++

WWW = /var/www/dev/rasm/boxcutter/download

CFLAGS=-mwindows -lcomctl32 -lgdi32 -I/usr/include/wine/msvcrt

all: boxcutter.exe boxcutter-fs.exe

boxcutter.exe: boxcutter.cpp
	$(CC) boxcutter.cpp -o boxcutter $(CFLAGS)

boxcutter-fs.exe: boxcutter-fs.cpp
	$(CC) boxcutter-fs.cpp -o boxcutter-fs $(CFLAGS)

png.exe: png.cpp
	$(CC) png.cpp -o png $(CFLAGS) -Lgdi -lgdiplus


pkg:
	mkdir -p dist
	rm -rf dist/boxcutter-$(VERSION)
	mkdir -p dist/boxcutter-$(VERSION)
	cp $(FILES) dist/boxcutter-$(VERSION)
	zip -r dist/boxcutter-$(VERSION).zip dist/boxcutter-$(VERSION)

upload:
	cp -r dist/boxcutter-$(VERSION).zip \
              dist/boxcutter-$(VERSION) $(WWW)

clean:
	rm -f boxcutter.exe boxcutter-fs.exe



