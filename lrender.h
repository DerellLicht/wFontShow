//**************************************************************************
//  Copyright (c)  2014  Daniel D Miller
//  home.comcast.net/~derelict
//  This program is freeware and can be used for any purpose,
//  commercial or otherwise, without contacting the author.
//  
//  The lrender class is derived from fontmgr class;
//  This class handles rendering of the font on a Windows display
//**************************************************************************

//lint -esym(1714, lrender::set_object_origin, lrender::get_string_height)
//lint -esym(1714, lrender::write_led_string, lrender::write_led_string, lrender::set_frame_attr)
//lint -esym(1714, lrender::set_frame_dimens, lrender::setfocus, lrender::draw_frame)
//lint -esym(1714, lrender::get_dimens, lrender::read_file, lrender::lrender)

//lint -esym(1740, lrender::hwndParent, lrender::hdcParent)

//lint -esym(1790, fontmgr) Base class 'fontmgr' has no non-destructor virtual functions

//  set_options() constants
#define  ROUND_PIXELS   0
#define  SQUARE_PIXELS  1
#define  WHOLE_SPACE    0
#define  HALF_SPACE     1

//lint -esym(756, lrender_init_t)
//lint -esym(1714, lrender::set_clipping_region)

//  maximum pixel dimensions
static uint const MAX_DIAMETER = 5 ;
static uint const MAX_BIT_GAP = 3 ;

typedef struct lrender_init_s {
   char   *font_name ;        //  Name of font file to use for display
   uint   diameter ;          //  diameter of pixels in font
   uint   bit_gap ;           //  gap between pixels in character
   uint   char_gap ;          //  gap between characters
   // uint nspace ;           //  half-space doesn't work 
   uint   pixel_type ;        //  square or round pixels
   COLORREF bgnd ;            //  background color
   COLORREF set_bit ;         //  color of set bits
   COLORREF clear_bit ;       //  color of cleared bits
} lrender_init_t, *lrender_init_p ;

/************************************************************************/
typedef struct char_frame_s {
   bool is_frame_valid ;
   HDC     hMemDC ;  //  draw on this
   HBITMAP hBmp   ;
} char_frame_t, *char_frame_p ;

/************************************************************************/
class lrender : public fontmgr {
private:
   HWND hwndParent ;
   HDC  hdcParent ;
   
   unsigned xleft ;
   unsigned ytop ;
   unsigned on_attr ;
   unsigned off_attr ;
   unsigned bg_attr ;
   unsigned diameter ;
   unsigned bit_gap ;
   unsigned char_gap ;  //  gap between characters
   unsigned narrow_space ;
   unsigned pixel_type ;   //  0=round, 1=square
   // unsigned frame_padding ;
   char_frame_t frames[256] ; //  one frame for each ASCII character

   //  private functions for internal class use
   void clear_frame(HDC hdc, unsigned dx, unsigned dy) const ;
   void draw_square(HDC hdc, unsigned x, unsigned y, unsigned diameter, COLORREF Color) const ;         // was lcommon.cpp
   void draw_circle(HDC hdc, unsigned x, unsigned y, unsigned diameter, COLORREF attr) const ;// was lcommon.cpp
   void draw_circle(HDC hdc, unsigned x, unsigned y, COLORREF attr);
   void draw_square(HDC hdc, int x, int y, COLORREF Color);
   void draw_element(HDC hdc, unsigned xi, unsigned yi, COLORREF attr);

//lint -esym(1719, lrender)  assignment operator for class 'lrender' has non-reference parameter
//lint -esym(1720, lrender)  assignment operator for class 'lrender' has non-const parameter
//lint -esym(1722, lrender)  assignment operator for class 'lrender' does not return a reference to class
//lint -esym(1704, lrender::lrender)  Constructor has private access specification
   //  disable copy and assignment operators
   lrender operator=(const lrender src) ;
   lrender(const lrender&);

public:
   //lint -esym(1712, lrender)
   lrender(HWND hwnd, char *fname) ;
   lrender(HWND hwnd, lrender_init_p init_data) ;
   ~lrender();
   void set_clipping_region(void); 
   void set_object_origin(unsigned x, unsigned y) ;
   void set_attr(unsigned fg, unsigned bg, unsigned bgnd);
   void set_dimens(unsigned new_diameter, unsigned new_bit_gap, unsigned char_gap) ;
   void set_options(unsigned new_nspace, unsigned new_pixel_type) ;
   unsigned write_led_char(unsigned x, unsigned y, u8 chr);
   void write_led_string(char *str, unsigned x, unsigned y);
   void write_led_string(char *msg);
   void reset_frames(void);
   int read_file(char *fname);

   void get_dimens(unsigned *curr_diameter, unsigned *curr_bit_gap, unsigned *curr_char_gap) const {
      *curr_diameter = diameter ;
      *curr_bit_gap  = bit_gap ;
      *curr_char_gap = char_gap ;
   } ;
   unsigned get_char_width(void) const {
      return (get_font_width() * diameter) + ((get_font_width()-1) * bit_gap) ;
   }
   unsigned get_char_height(void) const {
      return (get_font_points() * diameter) + ((get_font_points()-1) * bit_gap) ;
   }
} ;

