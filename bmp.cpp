

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
