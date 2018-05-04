//******************************************************************
//  Copyright (c)  2014  Daniel D Miller
//  home.comcast.net/~derelict
//  This program is freeware and can be used for any purpose,
//  commercial or otherwise, without contacting the author.
//  
//  The fontmgr class reads a bit-mapped raster font into
//  an internal storage area for further manipulation.
//******************************************************************
#include <windows.h>
#include <io.h>
#include <fcntl.h>
using namespace std;

#include "common.h"
#include "fontmgr.h"

// #include "font5x7.cxx"

//lint -esym(1566, fontmgr::user_font)

//******************************************************************
fontmgr::fontmgr(char *fname) :
   user_points(0),
   fwidth(5)
{
   read_font_file(fname) ;
}

//******************************************************************
int fontmgr::read_font_file(char *fname)
{
   // return read_dos_font(fname) ;
   u8 font_bfr[256 * 20];
   unsigned fchar, frow ;

   ZeroMemory(user_font, sizeof(user_font)) ;

   int fhdl = _open(fname, O_RDONLY | O_BINARY) ;
   if (fhdl < 0) {
      syslog("%s open: %s\n", fname, get_system_message()) ;
      return -1 ;
   }
   int rbytes = _read(fhdl, font_bfr, sizeof(font_bfr)) ;
   if (rbytes < 0) {
      syslog("%s read: %s\n", fname, get_system_message()) ;
      return -1 ;
   }

   //  verify file length
   if ((rbytes % 256) != 0) {
      syslog("%s read: bad size (%d), not MOD256\n", fname, rbytes) ;
      return -1 ;
   }
   user_points = rbytes / 256;
   _close(fhdl) ;

   //  read font into our user font table
   u8 *fptr = &font_bfr[0];
   for (fchar = 0; fchar < 256; fchar++) {
      for (frow = 0; frow < user_points; frow++) {
         user_font[fchar][frow] = *fptr++;
      }
   }
   // wsprintf(errstr, "%s: %u points\n", fname, user_points) ;
   // OutputDebugString(errstr) ;
   return 0;
}

//************************************************************************
u8 *fontmgr::get_char_font_map(char chr)
{
   u8 *uptr = &user_font[(unsigned) (u8) chr][0] ;
   return uptr ;
}

