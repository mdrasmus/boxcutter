  
  boxcutter
  Copyright Matt Rasmussen 2008

boxcutter is a simple command line-driven screenshot program.  Unlike
using the "Print Screen" key, boxcutter allows the user to screenshot
a smaller portion of the screen by dragging box.   boxcutter can also be 
executed NOT from the command line, in which case it will by default place a 
screenshot onto the clipboard.  

usage: boxcutter [OPTIONS] [OUTPUT_FILENAME]

Saves a bitmap screenshot to 'OUTPUT_FILENAME' if given.  Otherwise, 
screenshot is stored on clipboard by default.

OPTIONS
  -c, --coords X1,Y1,X2,Y2    capture the rectange (X1,Y1)-(X2,Y2)
  -h, --help                  display help message

