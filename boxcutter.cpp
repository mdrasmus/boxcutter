/*=============================================================================

  boxcutter
  Copyright Matt Rasmussen 2008

  A simple command line-driven screenshot program
  

=============================================================================*/

// this definition is needed for AttachConsole
#define _WIN32_WINNT 0x0501


// c includes
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include <iostream>
#include <fstream>

// windows includes
#include <windows.h>
#include <wincon.h>
#include <commctrl.h>
#include <winuser.h>
#include <io.h>
#include <conio.h>
#include <fcntl.h>

#define BOX_VERSION "1.3"

// constants
const char* g_usage = "\n\
usage: boxcutter [OPTIONS] [OUTPUT_FILENAME]\n\
Saves a bitmap screenshot to 'OUTPUT_FILENAME' if given.  Otherwise, \n\
screenshot is stored on the clipboard by default.\n\
\n\
OPTIONS\n\
  -c, --coords X1,Y1,X2,Y2    capture the rectange (X1,Y1)-(X2,Y2)\n\
  -f, --fullscreen            capture the full screen\n\
  -v, --version               display version information\n\
  -h, --help                  display help message\n\
";

const char* g_version = "\n\
boxcutter %s\n\
Copyright Matt Rasmussen 2008\n\
";

const char *g_class_name = "BoxCutter";

// globals
class BoxCutterWindow;
BoxCutterWindow *g_win = NULL;



//=============================================================================
// functions

/* Saves a bitmap to a file

   The following function was adopted from pywin32, and is thus under the
following copyright:

  Copyright (c) 1994-2008, Mark Hammond 
  All rights reserved.

  Redistribution and use in source and binary forms, with or without 
  modification, are permitted provided that the following conditions 
  are met:

  Redistributions of source code must retain the above copyright notice, 
  this list of conditions and the following disclaimer.

  Redistributions in binary form must reproduce the above copyright 
  notice, this list of conditions and the following disclaimer in 
  the documentation and/or other materials provided with the distribution.

  Neither name of Mark Hammond nor the name of contributors may be used 
  to endorse or promote products derived from this software without 
  specific prior written permission. 

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS
  IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 

*/
bool save_bitmap_file(HBITMAP hBmp, HDC hDC, const char *filename)
{
    // data structures
    BITMAP bmp;
    PBITMAPINFO pbmi;
    WORD cClrBits;

    // Retrieve the bitmap's color format, width, and height. 
    if (!GetObject(hBmp, sizeof(BITMAP), (LPVOID) &bmp))
        // GetObject failed
        return false;
    
    // Convert the color format to a count of bits. 
    cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel); 
    if (cClrBits == 1) 
      cClrBits = 1; 
    else if (cClrBits <= 4) 
      cClrBits = 4; 
    else if (cClrBits <= 8) 
      cClrBits = 8; 
    else if (cClrBits <= 16) 
      cClrBits = 16; 
    else if (cClrBits <= 24) 
      cClrBits = 24; 
    else cClrBits = 32; 

    
    // Allocate memory for the BITMAPINFO structure. (This structure 
    // contains a BITMAPINFOHEADER structure and an array of RGBQUAD 
    // data structures.) 
    if (cClrBits != 24) 
        pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
                                        sizeof(BITMAPINFOHEADER) + 
                                        sizeof(RGBQUAD) * (1<< cClrBits)); 

    // There is no RGBQUAD array for the 24-bit-per-pixel format. 
    else
        pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
                                        sizeof(BITMAPINFOHEADER)); 
  
    // Initialize the fields in the BITMAPINFO structure. 

    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
    pbmi->bmiHeader.biWidth = bmp.bmWidth; 
    pbmi->bmiHeader.biHeight = bmp.bmHeight; 
    pbmi->bmiHeader.biPlanes = bmp.bmPlanes; 
    pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel; 
    
    if (cClrBits < 24) 
        pbmi->bmiHeader.biClrUsed = (1<<cClrBits); 

    // If the bitmap is not compressed, set the BI_RGB flag. 
    pbmi->bmiHeader.biCompression = BI_RGB; 

    // Compute the number of bytes in the array of color 
    // indices and store the result in biSizeImage. 
    pbmi->bmiHeader.biSizeImage = (pbmi->bmiHeader.biWidth + 7) /8 
        * pbmi->bmiHeader.biHeight * cClrBits; 

    // Set biClrImportant to 0, indicating that all of the 
    // device colors are important. 
    pbmi->bmiHeader.biClrImportant = 0; 
  
    HANDLE hf;                  // file handle 
    BITMAPFILEHEADER hdr;       // bitmap file-header 
    PBITMAPINFOHEADER pbih;     // bitmap info-header 
    LPBYTE lpBits;              // memory pointer 
    DWORD dwTotal;              // total count of bytes 
    DWORD cb;                   // incremental count of bytes 
    BYTE *hp;                   // byte pointer 
    DWORD dwTmp; 

    pbih = (PBITMAPINFOHEADER) pbmi; 
    lpBits = (LPBYTE) GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);
    
    
    
    if (!lpBits) {
        // GlobalAlloc failed
        printf("error: out of memory\n");
        return false;
    }

    // Retrieve the color table (RGBQUAD array) and the bits 
    // (array of palette indices) from the DIB. 
    if (!GetDIBits(hDC, hBmp, 0, (WORD) pbih->biHeight, lpBits, pbmi, 
                   DIB_RGB_COLORS)) 
    {
        // GetDIBits failed
        printf("error: GetDiBits failed\n");
        return false;
    }

    // Create the .BMP file. 
    hf = CreateFile(filename, 
                    GENERIC_READ | GENERIC_WRITE, 
                    (DWORD) 0, 
                    NULL, 
                    CREATE_ALWAYS, 
                    FILE_ATTRIBUTE_NORMAL, 
                    (HANDLE) NULL); 
    if (hf == INVALID_HANDLE_VALUE) {
        // create file
        printf("error: cannot create file '%s'\n", filename);
        return false;
    }
    hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M" 
    // Compute the size of the entire file. 
    hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + 
                          pbih->biSize + pbih->biClrUsed 
                          * sizeof(RGBQUAD) + pbih->biSizeImage); 
    hdr.bfReserved1 = 0; 
    hdr.bfReserved2 = 0; 

    // Compute the offset to the array of color indices. 
    hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + 
        pbih->biSize + pbih->biClrUsed * sizeof (RGBQUAD); 

    // Copy the BITMAPFILEHEADER into the .BMP file. 
    if (!WriteFile(hf, (LPVOID) &hdr, sizeof(BITMAPFILEHEADER), 
                   (LPDWORD) &dwTmp,  NULL)) 
    {
        // WriteFile failed
        printf("error: cannot write file '%s'\n", filename);
        return false;
    }

    // Copy the BITMAPINFOHEADER and RGBQUAD array into the file. 
    if (!WriteFile(hf, (LPVOID) pbih, sizeof(BITMAPINFOHEADER) 
                   + pbih->biClrUsed * sizeof (RGBQUAD), 
                   (LPDWORD) &dwTmp, ( NULL)))
    {
        // WriteFile failed
        printf("error: cannot write file '%s'\n", filename);
        return false;
    }

    
    // Copy the array of color indices into the .BMP file. 
    dwTotal = cb = pbih->biSizeImage; 
    hp = lpBits; 
    if (!WriteFile(hf, (LPSTR) hp, (int) cb, (LPDWORD) &dwTmp, NULL)) {
        // WriteFile failed
        printf("error: cannot write file '%s'\n", filename);
        return false;
    }
    
    
    // Close the .BMP file. 
    if (!CloseHandle(hf)) {
        // CloseHandle failed
        printf("error: cannot close file '%s'\n", filename);
        return false;
    }
    
    // Free memory. 
    GlobalFree((HGLOBAL)lpBits);

    return true;
}
// End of Mark Hammond copyrighted code.


// Using swaps, ensure that x2 >= x, y2 >= y for capturing a rectangle of the
// screen.
void normalize_coords(int *x, int *y, int *x2, int *y2)
{
    if (*x > *x2) {
        int tmp = *x;
        *x = *x2;
        *x2 = tmp;
    }
    if (*y > *y2) {
        int tmp = *y;
        *y = *y2;
        *y2 = tmp;
    }
}


void GetScreenRect(RECT *rect)
{
    //GetWindowRect(GetDesktopWindow(), rect);
    rect->left = GetSystemMetrics(SM_XVIRTUALSCREEN);
    rect->top = GetSystemMetrics(SM_YVIRTUALSCREEN);
    rect->right = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    rect->bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN);
}



// Captures a screenshot from a region of the screen
// saves it to a file
bool capture_screen(const char *filename, int x, int y, int x2, int y2)
{
    // normalize coordinates
    normalize_coords(&x, &y, &x2, &y2);
    int w = x2 - x;
    int h = y2 - y;

    // copy screen to bitmap
    HDC screen_dc = GetDC(0);
    HDC shot_dc = CreateCompatibleDC(screen_dc);
    HBITMAP shot_bitmap =  CreateCompatibleBitmap(screen_dc, w, h);
    HGDIOBJ old_obj = SelectObject(shot_dc, shot_bitmap);
    
    if (!BitBlt(shot_dc, 0, 0, w, h, screen_dc, x, y, SRCCOPY)) {
	printf("error: BitBlt failed\n");
        return false;
    }
    
    // save bitmap to file
    bool ret = save_bitmap_file(shot_bitmap, shot_dc, filename);
    
    DeleteDC(shot_dc);
    DeleteDC(screen_dc);
    SelectObject(shot_dc, old_obj);
    
    return ret;
}


// Captures a screenshot from a region of the screen
// saves it to the clipboard
bool capture_screen_clipboard(HWND hwnd, int x, int y, int x2, int y2)
{
    // normalize coordinates
    normalize_coords(&x, &y, &x2, &y2);
    int w = x2 - x;
    int h = y2 - y;

    // copy screen to bitmap
    HDC screen_dc = GetDC(0);
    HDC shot_dc = CreateCompatibleDC(screen_dc);
    HBITMAP shot_bitmap =  CreateCompatibleBitmap(screen_dc, w, h);
    HGDIOBJ old_obj = SelectObject(shot_dc, shot_bitmap);
    
    if (!BitBlt(shot_dc, 0, 0, w, h, screen_dc, x, y, SRCCOPY)) {
        printf("error: BitBlt failed\n");
        return false;
    }
    
    // save bitmap to clipboard
    bool ret = false;
    if (OpenClipboard(hwnd)) {
        if (EmptyClipboard()) {
            if (SetClipboardData(CF_BITMAP, shot_bitmap))
                ret = true;
        }
        CloseClipboard();
    } else {
        printf("error: could not open clipboard\n");
    }

    // clean up
    DeleteDC(shot_dc);
    DeleteDC(screen_dc);
    SelectObject(shot_dc, old_obj);
    
    return ret;
}


//=============================================================================
// Window class for manual screenshot   

class BoxCutterWindow
{
public:
    BoxCutterWindow(HINSTANCE hinst, 
                    const char *title, const char* filename) :
        m_active(true),
        m_drag(false),
        m_draw(false),
        m_filename(filename),
        m_have_coords(false)
    {
        // fill wndclass structure
        WNDCLASS wc;
        strcpy(m_class_name, g_class_name);
        wc.hInstance = hinst; 
        wc.lpszClassName = m_class_name;
        wc.lpszMenuName = "";
        wc.lpfnWndProc = WindowProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.hbrBackground = 0; //(HBRUSH) GetStockObject(WHITE_BRUSH);
        wc.hCursor = LoadCursor(0, IDC_CROSS);
        wc.hIcon = LoadIcon(0, IDI_APPLICATION);
        
        // register class
        ATOM class_atom = RegisterClass(&wc);
        
        // determine screen dimensions
        RECT rect;
        GetScreenRect(&rect);
        
        // create window
        DWORD exstyle = WS_EX_TRANSPARENT;
        DWORD style = WS_POPUP;
        
        // set this window as the receiver of messages
        g_win = this;
        
        m_handle = CreateWindowEx(exstyle,
                                  m_class_name, 
                                  title,
                                  style,
                                  // dimensions
                                  0, 0, rect.right, rect.bottom,
                                  0, // no parent
                                  0, // no menu
                                  hinst, //module_instance,
                                  NULL);
    }
    
    ~BoxCutterWindow()
    {
    }

    //===============================
    // functions to manipulate window
    
    void show(bool enabled=true)
    {
        if (enabled)
            ShowWindow(m_handle, SW_SHOW);
        else
            ShowWindow(m_handle, SW_HIDE);
    }
    
    void maximize()
    {
        ShowWindow(m_handle, SW_SHOWMAXIMIZED);
        //UpdateWindow(m_handle);
    }
    
    void activate()
    {
        SetForegroundWindow(m_handle);
        //SwitchToThisWindow(self._handle, False)
    }
    
    
    void close()
    {
        DestroyWindow(m_handle);
    }    
    
    //=============================
    // accessors

    bool active()
    {
        return m_active;
    }

    HWND get_handle()
    {
        return m_handle;
    }

    void get_coords(int *x1, int *y1, int *x2, int *y2) 
    {
        *x1 = m_start.x;
        *y1 = m_start.y;
        *x2 = m_end.x;
        *y2 = m_end.y;
    }

    bool have_coords()
    {
        return m_have_coords;
    }

    //=======================================
    // event callbacks
    static LRESULT CALLBACK WindowProc(HWND hwnd,
                                       UINT uMsg,
                                       WPARAM wParam,
                                       LPARAM lParam)
    {
        // if window is present
        if (g_win) {
            // route message
        
            switch (uMsg) {
                case WM_DESTROY: 
                    g_win->close();
                    PostQuitMessage(0);
                    return 0;
            
                case WM_MOUSEMOVE:
                    g_win->on_mouse_move();
                    return 0;
                
                case WM_LBUTTONDOWN: 
                    g_win->on_mouse_down();
                    return 0;
                
                case WM_LBUTTONUP:
                    g_win->on_mouse_up();
                    return 0;
            }
        }
        
        // perform default action
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }


    // mouse button down callback
    void on_mouse_down()
    {
        // start draging
        m_drag = true;
        GetCursorPos(&m_start);
    }
    
    // mouse button up callback
    void on_mouse_up()
    {
        
        // if drawing has occurred, clean it up
        if (m_draw) {
            // cleanup rectangle on desktop
            m_drag = false;
            m_draw = false;
            
            HDC hdc = CreateDC("DISPLAY", NULL, NULL, NULL);
            SetROP2(hdc, R2_NOTXORPEN);
            Rectangle(hdc, m_start.x, m_start.y, m_end.x, m_end.y);
            DeleteDC(hdc);

            m_have_coords = true;
        }
        
        // stop BoxCutter window
        m_active = false;
    }
    
    // callback for mouse movement
    void on_mouse_move()
    {
        // get current mouse coordinates
        POINT pos;
        GetCursorPos(&pos);
        
        // if mouse is down, process drag
        if (m_drag) {
            HDC hdc = CreateDC("DISPLAY", NULL, NULL, NULL);
            SetROP2(hdc, R2_NOTXORPEN);
            
            // erase old rectangle
            if (m_draw) {
                Rectangle(hdc, m_start.x, m_start.y, m_end.x, m_end.y);
            }
            
            // draw new rectangle
            m_draw = true;
            Rectangle(hdc, m_start.x, m_start.y, pos.x, pos.y);
            m_end = pos;
            
            DeleteDC(hdc);
        }
    }


private:    
    char m_class_name[101];
    HWND m_handle;
    
    bool m_active;
    bool m_drag;
    bool m_draw;
    const char *m_filename;
    POINT m_start, m_end;
    bool m_have_coords;
};


//=============================================================================

// Display usage information
void usage()
{
    printf(g_usage);
}

// Display version information
void version()
{
    printf(g_version, BOX_VERSION);
}


// Pump the message queue for this process.
int main_loop(BoxCutterWindow* win)
{
    int ret;
    MSG msg; // message structure
    while ((ret = GetMessage(&msg, 0, 0, 0)) != 0 && win->active())
    {
        if (ret == -1) {
            // error occurred
            break;
        }
        
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

// Setup the console for output (e.g. using printf)
bool setup_console()
{
    int hConHandle;
    long lStdHandle;
    CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;

    // create a console
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
        // if no parent console then give up        
        return false;
    }
    
    
	const unsigned int MAX_CONSOLE_LINES = 500;
	// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),	&coninfo);
	coninfo.dwSize.Y = MAX_CONSOLE_LINES;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),	coninfo.dwSize);

    
	// redirect unbuffered STDOUT to the console
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);    
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen(hConHandle, "w");    
    if (!fp) {
        // could not open stdout
        return false;
    }
	*stdout = *fp;
	setvbuf(stdout, NULL, _IONBF, 0);
    
    
	// redirect unbuffered STDIN to the console
	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen(hConHandle, "r");
    if (!fp) {
        // could not open stdin
        return false;
    }
	*stdin = *fp;
	setvbuf(stdin, NULL, _IONBF, 0);

    
	// redirect unbuffered STDERR to the console
	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen(hConHandle, "w");
    if (!fp) {
        // could not open stderr
        return false;
    }
	*stderr = *fp;
	setvbuf(stderr, NULL, _IONBF, 0);

	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
	// point to console as well
	std::ios::sync_with_stdio();

    return true;
}



//=============================================================================
// main function


int main(int argc, char **argv)
{
    HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);

    InitCommonControls();
    setup_console();
    
    // default screenshot filename
    char *filename = NULL;

    // coordinates
    bool use_coords = false;
    int x1, y1, x2, y2;
    
    // parse command line
    int i;

    // parse options
    for (i=1; i<argc; i++) {
        if (argv[i][0] != '-')
            // argument is not an option
            break;

        else if (strcmp(argv[i], "-f") == 0 ||
                 strcmp(argv[i], "--fullscreen") == 0) 
        {
            RECT rect;
            GetScreenRect(&rect);
            x1 = rect.left;
            y1 = rect.top;
            x2 = rect.right;
            y2 = rect.bottom;
            use_coords = true;
        }
        
        else if (strcmp(argv[i], "-c") == 0 ||
                 strcmp(argv[i], "--coords") == 0) 
        {
            if (i+1 >= argc) {
                printf("error: expected argument for -c,--coord\n");
                usage();
                return 1;
            }
            
            if (sscanf(argv[++i], "%d,%d,%d,%d", &x1, &y1, &x2, &y2) != 4) {
                printf("error: expected 4 comma separated integers\n");
                usage();
                return 1;
            }
            
            use_coords = true;
        }
        
        else if (strcmp(argv[i], "-v") == 0 ||
                 strcmp(argv[i], "--version") == 0)
        {
            // display version information
            version();
            return 1;
        }

        else if (strcmp(argv[i], "-h") == 0 ||
                 strcmp(argv[i], "--help") == 0)
        {
            // display help info
            usage();
            return 1;
        }

        else {
            printf("error: unknown option '%s'\n", argv[i]);
            usage();
            return 1;
        }
    }
    
    // argument after options is a filename
    if (i < argc)
        filename = argv[i];



    // create screenshot window
    BoxCutterWindow win(hInstance, "BoxCutter", filename);
    

    if (use_coords) {
        win.show();
    } else {
        // manually acquire coordinates
        win.show();
        win.maximize();
        win.activate();
        
        main_loop(&win);
        if (win.have_coords()) {
            win.get_coords(&x1, &y1, &x2, &y2);
        } else {
            printf("error: cannot retrieve screenshot coordinates\n");
            return 1;
        }
    }

    // display screenshot coords
    printf("screenshot coords: (%d,%d)-(%d,%d)\n", x1, y1, x2, y2);


    // save bitmap
    if (filename) {
        // save to file
        if (!capture_screen(filename, x1, y1, x2, y2))
        {
            MessageBox(win.get_handle(), "Cannot save screenshot", 
                       "Error", MB_OK);
            return 1;
        }

        printf("screenshot saved to file: %s\n", filename);
    } else {
        // save to clipboard
        if (!capture_screen_clipboard(win.get_handle(), x1, y1, x2, y2))
        {
            MessageBox(win.get_handle(), "Cannot save screenshot to clipboard", 
                       "Error", MB_OK);
            return 1;
        }

        printf("screenshot saved to clipboard.\n");
    }

    win.close();
    
    return 0;
}

