  
  boxcutter
  Copyright Matt Rasmussen 2008-2011

boxcutter is a simple command line-driven screenshot program.  Unlike
using the "Print Screen" key, boxcutter allows the user to screenshot
a smaller portion of the screen by dragging a box.   boxcutter can also be 
executed NOT from the command line, in which case it will by default place a 
screenshot onto the clipboard.  

usage: boxcutter [OPTIONS] [OUTPUT_FILENAME]

Saves a screenshot to 'OUTPUT_FILENAME' if given.  Only output formats
"*.bmp" and "*.png" are supported.  If no file name is given,
screenshot is stored on clipboard by default.

OPTIONS
  -c, --coords X1,Y1,X2,Y2    capture the rectange (X1,Y1)-(X2,Y2)
  -f, --fullscreen            capture the full screen\n\
  -h, --help                  display help message




  
  boxcutter-fs
  Copyright Matt Rasmussen 2008-2011
  Changes suggested by John Miller

boxcutter-fs is a minimal Windows command line screenshot
application. It requires and accepts a single argument: the file name
to store the screenshot. Nothing is echoed to the console. If the
program exits on 0, it was successful. If not, the program
failed. Screenshots are stored as uncompressed bitmaps.  This program is
useful for very quick fullscreen screenshots.

usage: boxcutter-fs OUTPUT_FILENAME

Saves a bitmap screenshot to 'OUTPUT_FILENAME'.
