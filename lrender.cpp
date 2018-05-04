//**************************************************************************
//  Copyright (c)  2014  Daniel D Miller
//  home.comcast.net/~derelict
//  This program is freeware and can be used for any purpose,
//  commercial or otherwise, without contacting the author.
//  
//  The lrender class is derived from fontmgr class;
//  This class handles rendering of the font on a Windows display
//**************************************************************************
#include <windows.h>

//lint -esym(1762, lrender::draw_element)

#include "common.h"
#include "fontmgr.h"
#include "lrender.h"

//************************************************************************
void lrender::clear_frame(HDC hdc, unsigned dx, unsigned dy) const 
{
   HBRUSH hBrush ;
   RECT   rect ;

   SetRect (&rect, 0, 0, dx, dy) ;
   hBrush = CreateSolidBrush (RGB(0,0,0)) ;
   FillRect (hdc, &rect, hBrush) ;
   DeleteObject (hBrush) ;
}

//************************************************************************
void lrender::draw_square(HDC hdc, unsigned x, unsigned y, unsigned diam, COLORREF Color) const 
{
   HBRUSH hBrush ;
   RECT   rect ;

   SetRect (&rect, x, y, x+diam, y+diam) ;
   hBrush = CreateSolidBrush (Color) ;
   FillRect (hdc, &rect, hBrush) ;
   DeleteObject (hBrush) ;
}

//***********************************************************************
void lrender::draw_circle(HDC hdc, unsigned x, unsigned y, unsigned diam, COLORREF attr) const 
{
   HPEN hPen = CreatePen(PS_SOLID, 1, attr) ;
   SelectObject(hdc, hPen) ;
   HBRUSH hBrush = CreateSolidBrush(attr) ;
   SelectObject(hdc, hBrush) ;
   Ellipse(hdc, x, y, x+diam, y+diam) ;
   DeleteObject (hBrush) ;
   SelectObject(hdc, GetStockObject(BLACK_PEN)) ;  //  deselect my pen
   DeleteObject (hPen) ;
}     

//**************************************************************************
lrender::lrender(HWND hwnd, char *fname) :
   fontmgr(fname),
   hwndParent(hwnd),
   hdcParent(GetDC(hwnd)),
   xleft(0),
   ytop(0),
   on_attr(RGB(255, 0, 0)),
   off_attr(RGB(63, 63, 63)),
   bg_attr(0),
   diameter(5),
   bit_gap(2),
   char_gap(2),
   narrow_space(1),
   pixel_type(1)
{
   ZeroMemory(frames, sizeof(frames)) ;
}

//**************************************************************************
lrender::lrender(HWND hwnd, lrender_init_p init_data) :
   fontmgr(init_data->font_name),
   hwndParent(hwnd),
   hdcParent(GetDC(hwnd)),
   xleft(0),
   ytop(0),
   on_attr(RGB(255, 0, 0)),
   off_attr(RGB(63, 63, 63)),
   bg_attr(0),
   diameter(5),
   bit_gap(2),
   char_gap(2),
   narrow_space(1),
   pixel_type(1)
{
   ZeroMemory(frames, sizeof(frames)) ;
   // test_led = new lrender(hwnd, font_name) ;
   // set_dimens(3, 1, 2) ;  //  diameter, bit gap, inter-char gap
   set_dimens(init_data->diameter, init_data->bit_gap, init_data->char_gap) ;
   // set_options(WHOLE_SPACE, SQUARE_PIXELS) ;
   set_options(WHOLE_SPACE, init_data->pixel_type) ;
   // set_attr(RGB(63, 181, 255), RGB(23, 64, 103), RGB(31, 31, 31)) ;
   set_attr(init_data->set_bit, init_data->clear_bit, init_data->bgnd) ;
}

/************************************************************************/
lrender::~lrender()
{
   uint idx ;
   for (idx=0; idx < 256; idx++) {
      char_frame_p fptr = &frames[idx] ;
      if (fptr->is_frame_valid) {
         DeleteDC(fptr->hMemDC);
         DeleteObject(fptr->hBmp);
         fptr->is_frame_valid = false ;
      }
   }
   ReleaseDC(hwndParent, hdcParent) ;
}

/************************************************************************/
void lrender::set_clipping_region(void)
{
   RECT r2;
   GetClientRect(hwndParent, &r2);
   HRGN clipregion = CreateRectRgnIndirect(&r2);
   SelectClipRgn(hdcParent, clipregion);
   DeleteObject(clipregion);
}

/************************************************************************/
void lrender::set_object_origin(unsigned x, unsigned y)
{
   xleft = x ;
   ytop = y ;
}

/************************************************************************/
void lrender::reset_frames(void)
{
   uint idx ;
   for (idx=0; idx < 256; idx++) {
      frames[idx].is_frame_valid = false ;
   }
}

//******************************************************************
void lrender::set_attr(unsigned fg, unsigned bg, unsigned bgnd)
{
   on_attr  = fg ;
   off_attr = bg ;
   bg_attr  = bgnd ;
   reset_frames() ;
}

//***********************************************************************
void lrender::set_dimens(unsigned new_diameter, unsigned new_bit_gap, unsigned new_char_gap) 
{
   diameter = new_diameter ;
   if (diameter == 0)
      diameter = 1 ;
   else if (new_diameter <= MAX_DIAMETER)
      diameter = new_diameter ;
   else
      diameter = MAX_DIAMETER ;
      

   bit_gap  = (new_bit_gap <= MAX_BIT_GAP) ? new_bit_gap : MAX_BIT_GAP ;
   char_gap = new_char_gap ;
   reset_frames() ;
}

//************************************************************************
void lrender::set_options(unsigned new_nspace, unsigned new_pixel_type) 
{
   narrow_space = (new_nspace) ? 1 : 0 ;
   pixel_type = (new_pixel_type) ? 1 : 0 ;
   reset_frames() ;
}  //lint !e715 enable_fixed_font

/************************************************************************/
int lrender::read_file(char *fname)
{
   reset_frames() ;
   return read_font_file(fname);
}

/************************************************************************/
void lrender::draw_element(HDC hdc, unsigned xi, unsigned yi, COLORREF attr)
{
   if (diameter == 1) 
      SetPixel(hdc, xi, yi, attr) ;
   else if (pixel_type == 0)
      draw_circle(hdc, xi, yi, diameter, attr) ;
   else
      draw_square(hdc, xi, yi, diameter, attr) ;
}

/************************************************************************/
unsigned lrender::write_led_char(unsigned x, unsigned y, u8 chr)
{
   unsigned bit_offset = diameter + bit_gap ;
   unsigned dy ;
   u8 *uptr ;
   unsigned xi ;
   // unsigned xpixels ;
   // unsigned yi = y + frame_padding ;
   unsigned yi = 0 ;
   // int frame_dx  = ( 8 * diameter) + ( 7 * bit_gap) ; //  these should be max values??
   // int frame_dy  = (19 * diameter) + (18 * bit_gap) ; //  these should be max values??
   int frame_dx  = ( 8 * MAX_DIAMETER) + ( 7 * MAX_BIT_GAP) ; //  these should be max values??
   int frame_dy  = (19 * MAX_DIAMETER) + (18 * MAX_BIT_GAP) ; //  these should be max values??
   int active_dy = (get_font_points() * diameter) + ((get_font_points()-1) * bit_gap) ;

   char_frame_p fptr = &frames[chr] ;

   if (!fptr->is_frame_valid) {
      if (fptr->hMemDC == 0)
          fptr->hMemDC = CreateCompatibleDC(hdcParent);
      if (fptr->hBmp == 0) 
          fptr->hBmp = CreateCompatibleBitmap(hdcParent, frame_dx, frame_dy);
      SelectObject(fptr->hMemDC, fptr->hBmp);      
      clear_frame(fptr->hMemDC, frame_dx, frame_dy) ;
      fptr->is_frame_valid = true ;

      uptr = get_char_font_map(chr) ;
      //  build the frame for this character, in the memory buffer
      for (dy=0; dy<get_font_points(); dy++) {
         //  draw one row
         u8 row_bits = *uptr++ ;
         u8 rmask = 0x80 ;
         if (chr == ' '  &&  narrow_space) {
            rmask = 0x08 ;
         }
         // xi = x + frame_padding ;
         xi = 0 ;
         while (rmask != 0) {
            // draw_element(hdcParent, xi, yi, (row_bits & rmask) ? on_attr : off_attr) ;
            draw_element(fptr->hMemDC, xi, yi, (row_bits & rmask) ? on_attr : off_attr) ;
            xi += bit_offset ;
            rmask >>= 1 ;
         }
         yi += bit_offset ;
         
      }
   }
   BitBlt(hdcParent, x, y, frame_dx, active_dy, fptr->hMemDC, 0, 0, SRCCOPY);

   //  once done, bitblt into screen buffer (hdcParent)
   return (chr == ' '  &&  narrow_space) ? 4 : 8 ;
}

/************************************************************************/
void lrender::write_led_string(char *str, unsigned x, unsigned y)
{
   while (*str != 0) {
      write_led_char(x, y, *str) ;
      str++ ;
      x += get_char_width() + char_gap ;
   }
}

/************************************************************************/
void lrender::write_led_string(char *msg)
{
   write_led_string(msg, xleft, ytop) ;
}

