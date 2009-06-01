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
CC=/c/mingw/bin/g++
    
WWW = /z/mnt/big/www/dev/rasm/boxcutter/download

CFLAGS=-mwindows -lcomctl32 -lgdi32 -I/usr/include/wine/msvcrt

all: boxcutter boxcutter-fs

boxcutter: boxcutter.cpp
	$(CC) boxcutter.cpp -o boxcutter $(CFLAGS)

boxcutter-fs: boxcutter-fs.cpp
	$(CC) boxcutter-fs.cpp -o boxcutter-fs $(CFLAGS)


pkg:
	mkdir -p dist
	rm -rf dist/boxcutter-$(VERSION)
	mkdir -p dist/boxcutter-$(VERSION)
	cp $(FILES) dist/boxcutter-$(VERSION)
	tar zcvf dist/boxcutter-$(VERSION).tar.gz dist/boxcutter-$(VERSION)
    
winupload:
	cp -r dist/boxcutter-$(VERSION).tar.gz \
        dist/boxcutter-$(VERSION) $(WWW)

clean:
	rm -f boxcutter.exe boxcutter-fs.exe



