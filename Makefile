# Makefile for boxcutter

boxcutter: boxcutter.cpp
	g++ boxcutter.cpp -o boxcutter -mwindows \
		-lcomctl32 -lgdi32

clean:
	rm -f boxcutter.exe