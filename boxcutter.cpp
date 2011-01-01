/*=============================================================================

  boxcutter
  Copyright Matt Rasmussen 2008-2011

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

#include "bmp.cpp"
#include "png.cpp"


#define BOX_VERSION "1.5"

// constants
const char* g_usage = "\n\
usage: boxcutter [OPTIONS] [OUTPUT_FILENAME]\n\
Saves a screenshot to 'OUTPUT_FILENAME' if given.  Only output formats\n\
'*.bmp' and '*.png' are supported.  If no file name is given,\n\
screenshot is stored on clipboard by default.\n\
\n\
OPTIONS\n\
  -c, --coords X1,Y1,X2,Y2    capture the rectange (X1,Y1)-(X2,Y2)\n\
  -f, --fullscreen            capture the full screen\n\
  -v, --version               display version information\n\
  -h, --help                  display help message\n\
";

const char* g_version = "\n\
boxcutter %s\n\
Copyright Matt Rasmussen 2008-2011\n\
";

const char *g_class_name = "BoxCutter";

// globals
class BoxCutterWindow;
BoxCutterWindow *g_win = NULL;



//=============================================================================
// functions



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


void get_screen_rect(RECT *rect)
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
    bool ret = false;
    
    int len = strlen(filename);
    if (len > 4 && strcasecmp(filename + len - 4, ".png") == 0) {
        ret = save_png_file(shot_bitmap, shot_dc, filename);
    } else if (len > 4 && strcasecmp(filename + len - 4, ".bmp") == 0) {
        ret = save_bitmap_file(shot_bitmap, shot_dc, filename);
    } else {
        printf("error: unknown output file format\n");
        ret = false;
    }
    
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
        get_screen_rect(&rect);
        
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
            get_screen_rect(&rect);
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

