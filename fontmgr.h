//******************************************************************
//  Copyright (c)  2014  Daniel D Miller
//  home.comcast.net/~derelict
//  This program is freeware and can be used for any purpose,
//  commercial or otherwise, without contacting the author.
//  
//  The fontmgr class reads a bit-mapped raster font into
//  an internal storage area for further manipulation.
//******************************************************************

class fontmgr {
private:
   u8 user_font[256][20];
   unsigned user_points ;
   unsigned fwidth ;   //  always 8 for normal font, but 5 for fixed font!

public:
   //lint -esym(1712, fontmgr)
   fontmgr(char *fname) ;
   virtual ~fontmgr() {} ;
   int read_font_file(char *fname);
   u8 *get_char_font_map(char chr);
   unsigned get_font_points(void) const 
      { return user_points; } ;
   unsigned get_font_width(void) const 
      { return 8; } ;
   
} ;

