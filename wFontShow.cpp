//**********************************************************************
//  Copyright (c) 2009-2014  Daniel D Miller
//  wShowFont.exe - Viewer for raster font files
//  
//  Written by:   Daniel D. Miller
//**********************************************************************
//  version		changes
//	 =======		======================================
//    1.00     original, derived from wFontList
//****************************************************************************

static char const * const Version = "wShowFont, Version 1.00" ;

#include "targetver.h"
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <sys/stat.h>
#include <time.h>
#include <commctrl.h>

#include "resource.h"
#include "common.h"
#include "statbar.h"
#include "fontmgr.h"
#include "lrender.h"
#ifdef  USE_WINMSGS
#include "winmsgs.h"
#endif

//  tooltips.cpp
extern HWND create_tooltips(HWND hwnd, uint max_width, uint popup_msec, uint stayup_msec);
extern void add_program_tooltips(HWND hwnd, HWND hwndToolTip);

//lint -esym(526, atoi)
//lint -esym(628, atoi)
//lint -esym(746, atoi)
//lint -esym(1055, atoi)

//lint -esym(715, hwnd, private_data, message, wParam, lParam)
//***********************************************************************

static uint main_timer_id = 0 ;

static CStatusBar *MainStatusBar = NULL;

static uint cxClient = 0;
static uint cyClient = 0;   //  subtrace height of status bar

static bool init_done = false ;
static bool handle_restore = false ;

//lint -esym(844, hwndBitGapSpin, hwndCharGapSpin, hwndPixDiamSpin)
static HWND hwndFontName    = NULL ;
static HWND hwndPixDiam     = NULL ;
static HWND hwndBitGap      = NULL ;
static HWND hwndCharGap     = NULL ;
static HWND hwndPixDiamSpin = NULL ;
static HWND hwndBitGapSpin  = NULL ;
static HWND hwndCharGapSpin = NULL ;
static HWND hwndSetAttr     = NULL ;
static HWND hwndClearAttr   = NULL ;
static HWND hwndBgndAttr    = NULL ;
static HWND hwndShowFont    = NULL ;

static char font_name[1024] = "script2.f19" ;
// static char font_name[1024] = "fntcol\\readable.f08" ;
static lrender *test_led ;

// static uint const MAX_DIAMETER = 3 ;   //  from lrender.h
// static uint const MAX_BIT_GAP  = 3 ;   //  from lrender.h
static uint const MAX_CHAR_GAP = 6 ;

static lrender_init_t test_init = {
   font_name, 3, 1, 2, SQUARE_PIXELS, RGB(31, 31, 31), RGB(63, 181, 255), RGB(23, 64, 103)
} ;

//***********************************************************************
static uint screen_width  = 0 ;
static uint screen_height = 0 ;

static void get_monitor_dimens(HWND hwnd)
{
   HMONITOR currentMonitor;      // Handle to monitor where fullscreen should go
   MONITORINFO mi;               // Info of that monitor
   currentMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
   mi.cbSize = sizeof(MONITORINFO);
   if (GetMonitorInfo(currentMonitor, &mi) != FALSE) {
      screen_width  = mi.rcMonitor.right  - mi.rcMonitor.left ;
      screen_height = mi.rcMonitor.bottom - mi.rcMonitor.top ;
   }
}

static uint get_screen_width(void)
{
   return screen_width ;
}

static uint get_screen_height(void)
{
   return screen_height ;
}

//***********************************************************************
void show_statbar_msg(char *msg)
{
   // StatusBar_SetBgndColor(hwndStatusBar, RGB(0, 255, 0)) ;
   // StatusBar_SetText(hwndStatusBar, 0, 0, msg);
   MainStatusBar->show_message(msg);
}

//***********************************************************************
//lint -esym(714, show_statbar_msg)
//lint -esym(759, show_statbar_msg)
//lint -esym(765, show_statbar_msg)
void show_statbar_msg(uint idx, char *msg)
{
   // StatusBar_SetBgndColor(hwndStatusBar, RGB(0, 255, 0)) ;
   // StatusBar_SetText(hwndStatusBar, 0, 0, msg);
   MainStatusBar->show_message(idx, msg);
}

//***********************************************************************
void show_statbar_msg(char *msg, COLORREF bgnd)
{
   MainStatusBar->set_bgnd_color(bgnd);
   // StatusBar_SetText(hwndStatusBar, 0, 0, msg);
   MainStatusBar->show_message(msg);
}

//**************************************************************************
//  remove static, to get rid of compiler warning about unused function.
//  I want to keep this function present, for reference purposes.
//**************************************************************************
//lint -esym(714, show_statbar_msgf)
//lint -esym(759, show_statbar_msgf)
//lint -esym(765, show_statbar_msgf)
int show_statbar_msgf(const char *fmt, ...)
{
   char consoleBuffer[260] ;
   va_list al; //lint !e522

   va_start(al, fmt);   //lint !e1055 !e530
   vsprintf(consoleBuffer, fmt, al);   //lint !e64
   show_statbar_msg(consoleBuffer);
   va_end(al);
   return 1;
}

//********************************************************************
static int show_statbar_msgfn(uint idx, const char *fmt, ...)
{
   char consoleBuffer[260] ;
   va_list al; //lint !e522

   va_start(al, fmt);   //lint !e1055 !e530
   vsprintf(consoleBuffer, fmt, al);   //lint !e64
   show_statbar_msg(idx, consoleBuffer);
   va_end(al);
   return 1;
}

/****************************************************************************
 * This hook procedure, which allows ClearIcon to position the color dialog
 * as desired, was extracted from Nancy Cluts, Chapter 06, cmndlg32.
 * The technique of setting the dialog position by trapping WM_INITDIALOG
 * in the hook procedure, was taken from comments on the web.
****************************************************************************/
#define  USE_CHOOSECOLOR_HOOK    1

#ifdef  USE_CHOOSECOLOR_HOOK
static BOOL APIENTRY ChooseColorHookProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
   switch (message) {
   case WM_INITDIALOG:
      {
      DWORD UFLAGS = SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW;
      SetWindowPos(hDlg, HWND_TOP, 400, 300, 0, 0, UFLAGS);
      }   
      break;
        
   //  Calling syslog() here, causes dialog to not close
   // case WM_COMMAND:
   //    if (LOWORD(wParam) == IDOK) {
   //       syslog("IDOK\n") ;
   //       return (TRUE);
   //    }
   //    else if (LOWORD(wParam) == IDCANCEL) {
   //       syslog("IDCANCEL\n") ;
   //       return TRUE;
   //    } else {
   //       syslog("0x%04X\n", LOWORD(wParam)) ;
   //    }
   //    break;
   }  //lint !e744 switch statement has no default
   return (FALSE);
}
#endif

//****************************************************************
static COLORREF select_color(HWND hwnd, COLORREF old_attr)
{
   static CHOOSECOLOR cc ;
   static COLORREF    crCustColors[16] ;

   // uint idx ;
   // for (idx=0; idx<16; idx++) {
   //    crCustColors[idx] = RGB(idx, idx, idx) ;
   // }

   ZeroMemory(&cc, sizeof(cc));
   cc.lStructSize    = sizeof (CHOOSECOLOR) ;
   cc.rgbResult      = old_attr ;
   cc.lpCustColors   = crCustColors ;
#ifdef  USE_CHOOSECOLOR_HOOK
   cc.Flags          = CC_RGBINIT | CC_FULLOPEN | CC_ENABLEHOOK;
   cc.lpfnHook = (LPCCHOOKPROC) ChooseColorHookProc;
   // cc.lpTemplateName = (LPTSTR) NULL;  //  not needed, struct was 0'd out
#else
   cc.Flags          = CC_RGBINIT | CC_FULLOPEN ;
#endif
   cc.hwndOwner      = hwnd ;

   if (ChooseColor(&cc) == TRUE) {
      return cc.rgbResult ;
   } else {
      return old_attr ;
   }
}  //lint !e715

//*********************************************************************
//  clear entire background of current field
//*********************************************************************
static void fill_color_field(HWND hwnd, COLORREF attr)
{
   RECT m_rect ;
   GetClientRect(hwnd, &m_rect);

   HDC hdc = GetDC(hwnd) ;
   HBRUSH hbBkBrush = CreateSolidBrush(attr);
   FillRect(hdc, &m_rect, hbBkBrush);
   DeleteObject(hbBkBrush);
   ReleaseDC(hwnd, hdc) ;
}

//*********************************************************************
//  draw the target objects
//*********************************************************************
static void update_display(void)
{
   // show_statbar_msgf("font: %s", font_name) ;
   fill_color_field(hwndShowFont,  test_init.bgnd) ;
   unsigned diameter, bit_gap, char_gap ;
   test_led->get_dimens(&diameter, &bit_gap, &char_gap) ;
   unsigned char_dx = test_led->get_char_width() ;
   unsigned char_dy = test_led->get_char_height() ;

   unsigned y_offset = 10 ;
   u8 idx = 0 ;
   unsigned row, col ;
   for (row=0; row<8; row++) {
      unsigned x_offset = 10 ;
      for (col=0; col<32; col++) {
         test_led->write_led_char(x_offset, y_offset, idx) ;
         x_offset += char_dx + char_gap;
         idx++ ;
      }
      y_offset += char_dy + char_gap;
   }
}

//************************************************************************
static char const szPalFilter[]  = "Font Files (*.F*)\0*.f*\0"  \
                                   "All Files (*.*)\0*.*\0\0" ;

static bool read_font_file(HWND hwnd)
{
   OPENFILENAME ofn;       // common dialog box structure
   char szFile[260];       // buffer for file name
   char oldFile[260];       // buffer for file name
   char dirFile[260];       // buffer for file name
   // HWND hwnd;              // owner window
   // HANDLE hf;              // file handle

   // Initialize OPENFILENAME
   ZeroMemory(&ofn, sizeof(ofn));
   ofn.lStructSize = sizeof(ofn);
   ofn.hwndOwner = hwnd;
   ofn.lpstrFile = szFile;
   //
   // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
   // use the contents of szFile to initialize itself.
   //
   ofn.lpstrFile[0] = '\0';
   strcpy(dirFile, font_name) ;
   char *strptr = strrchr(dirFile, '\\') ;
   if (strptr != 0) {
      strptr++ ;  //  leave the backslash in place
      *strptr = 0 ;  //  strip off filename
      // OutputDebugString(dirFile) ;
   }
   ofn.lpstrInitialDir = dirFile ;
   ofn.nMaxFile = sizeof(szFile);
   ofn.lpstrFilter = szPalFilter ;
   ofn.nFilterIndex = 1;
   ofn.lpstrTitle = "select font file" ;
   ofn.lpstrFileTitle = NULL ;
   ofn.lpstrDefExt = TEXT ("f*") ;
   // ofn.nMaxFileTitle = 0;
   // ofn.lpstrInitialDir = NULL;
   ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

   // Display the Open dialog box. 
   if (GetOpenFileName(&ofn) == TRUE) {
      strncpy(oldFile, font_name, sizeof(oldFile)) ;
      strncpy(font_name, ofn.lpstrFile, sizeof(font_name)) ;

      // test_led->set_options(WHOLE_SPACE, SQUARE_PIXELS) ;
      int result = test_led->read_file(font_name) ;
      if (result != 0) {
         // wsprintf(tempstr, "%s: %s", palette_filename, strerror(result)) ;
         syslog("%s: read failed", font_name) ;
         strncpy(font_name, oldFile, sizeof(font_name)) ;
         return false;
      }
      return true;
   }
   return false;
}

//****************************************************************************
static uint read_count(HWND hwnd)
{
   char tempbfr[11] ;
   GetWindowTextA (hwnd, tempbfr, 10);
   tempbfr[10] = 0 ;
   char *tptr = strip_leading_spaces(tempbfr) ;
   return (uint) atoi(tptr) ;
}

//*******************************************************************
static bool do_init_dialog(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LPVOID private_data)
{
   char msgstr[81] ;
   wsprintfA(msgstr, "%s", Version) ;
   SetWindowTextA(hwnd, msgstr) ;

   SetClassLongA(hwnd, GCL_HICON,   (LONG) LoadIcon(GetModuleHandle(NULL), (LPCSTR) LCDTICO));
   SetClassLongA(hwnd, GCL_HICONSM, (LONG) LoadIcon(GetModuleHandle(NULL), (LPCSTR) LCDTICO));
   get_monitor_dimens(hwnd);

   {  //  create local context
   RECT myRect ;
   GetClientRect(hwnd, &myRect) ;
   cxClient = (myRect.right - myRect.left) ;
   cyClient = (myRect.bottom - myRect.top) ;

   //****************************************************************
   //  create/configure status bar first
   //****************************************************************
   MainStatusBar = new CStatusBar(hwnd) ;
   MainStatusBar->MoveToBottom(cxClient, cyClient) ;

   //  re-position status-bar parts
   {
   int sbparts[3];
   sbparts[0] = (int) (6 * cxClient / 10) ;
   sbparts[1] = (int) (8 * cxClient / 10) ;
   sbparts[2] = -1;
   MainStatusBar->SetParts(3, &sbparts[0]);
   }
   }

   //*****************************************
   //  initialize controls
   //*****************************************
   CheckRadioButton (hwnd, IDC_RB_ROUND, IDC_RB_SQUARE, IDC_RB_ROUND + test_init.pixel_type) ;
   // show_statbar_msgf("pixel type = %s", (test_init.pixel_type == ROUND_PIXELS) ? "Round" : "Square") ;

   hwndFontName = GetDlgItem(hwnd, IDC_FONTNAME) ;
   wsprintf(msgstr, "%s", test_init.font_name) ;
   SetWindowText(hwndFontName, msgstr) ;

   //  manage pixel-diameter field
   hwndPixDiam = GetDlgItem(hwnd, IDC_PIXDIAM) ;
   hwndPixDiamSpin = CreateUpDownControl(
      WS_CHILD|WS_VISIBLE|UDS_SETBUDDYINT|UDS_ALIGNRIGHT|WS_BORDER,
      // spin button control style
      // UDS_SETBUDDYINT   Causes the control to set the text of the buddy window
      // UDS_ALIGNRIGHT or UDS_ALIGNLEFT  must be set or else updown will not place beside the edit
      0, 0, 0, 0,          // size and position
      hwnd,                // parent handle
      IDC_PIXDIAMSPIN,     // updown ID
      GetModuleHandle(NULL),             // instance handle
      hwndPixDiam,         // associated field, or NULL
      MAX_DIAMETER,        // max value
      1,                   // min value
      test_init.diameter); // initial value

   //  manage bit-gap field
   hwndBitGap  = GetDlgItem(hwnd, IDC_BITGAP) ;
   hwndBitGapSpin = CreateUpDownControl(
      WS_CHILD|WS_VISIBLE|UDS_SETBUDDYINT|UDS_ALIGNRIGHT|WS_BORDER,
      // spin button control style
      // UDS_SETBUDDYINT   Causes the control to set the text of the buddy window
      // UDS_ALIGNRIGHT or UDS_ALIGNLEFT  must be set or else updown will not place beside the edit
      0, 0, 0, 0,          // size and position
      hwnd,                // parent handle
      IDC_BITGAPSPIN,      // updown ID
      GetModuleHandle(NULL),             // instance handle
      hwndBitGap,          // associated field, or NULL
      MAX_BIT_GAP,         // max value
      0,                   // min value
      test_init.bit_gap);  // initial value

   //  manage char-gap field
   hwndCharGap = GetDlgItem(hwnd, IDC_CHARGAP) ;
   hwndCharGapSpin = CreateUpDownControl(
      WS_CHILD|WS_VISIBLE|UDS_SETBUDDYINT|UDS_ALIGNRIGHT|WS_BORDER,
      // spin button control style
      // UDS_SETBUDDYINT   Causes the control to set the text of the buddy window
      // UDS_ALIGNRIGHT or UDS_ALIGNLEFT  must be set or else updown will not place beside the edit
      0, 0, 0, 0,          // size and position
      hwnd,                // parent handle
      IDC_CHARGAPSPIN,     // updown ID
      GetModuleHandle(NULL),             // instance handle
      hwndCharGap,         // associated field, or NULL
      MAX_CHAR_GAP,        // max value
      0,                   // min value
      test_init.char_gap); // initial value

   //  fill in color fields
   hwndSetAttr   = GetDlgItem(hwnd, IDC_SHOW_SET) ;
   hwndClearAttr = GetDlgItem(hwnd, IDC_SHOW_CLEAR) ;
   hwndBgndAttr  = GetDlgItem(hwnd, IDC_SHOW_BGND) ;
   hwndShowFont  = GetDlgItem(hwnd, IDC_SHOW_FONT) ;

   HWND hToolTip = create_tooltips(hwnd, 150, 100, 10000) ;
   add_program_tooltips(hwnd, hToolTip) ;

   //  do some of the init work later, because it does not work now
   main_timer_id = SetTimer(hwnd, IDT_TIMER_MAIN, 100, (TIMERPROC) NULL) ;
   return true ;
}

//*******************************************************************
static bool do_timer(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LPVOID private_data)
{
   switch (wParam) {
   case IDT_TIMER_MAIN:
      if (main_timer_id != 0) {
         if (!init_done) {
            fill_color_field(hwndSetAttr,   test_init.set_bit) ;
            fill_color_field(hwndClearAttr, test_init.clear_bit) ;
            fill_color_field(hwndBgndAttr,  test_init.bgnd) ;

            test_led = new lrender(hwndShowFont, &test_init) ;
            test_led->set_clipping_region() ;
            update_display() ;
            show_statbar_msgfn(1, "screen size: %ux%u", get_screen_width(), get_screen_height());
            init_done = true ;
         } else if (handle_restore) {
            update_display() ;
            handle_restore = false ;
         }

         KillTimer(hwnd, main_timer_id) ;
         main_timer_id = 0 ;
      }
      break;

   default:
      break;
      // return DefWindowProcA(hwnd, iMsg, wParam, lParam);
   }
   return true ;
}

//*******************************************************************
static bool do_vscroll(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LPVOID private_data)
{
// #define SB_THUMBPOSITION   4
// #define SB_ENDSCROLL       8
// [3496] scroll code=4, nPos=2
// [3496] scroll code=8, nPos=2
//    syslog("scroll code=%u, nPos=%u\n", LOWORD(wParam), HIWORD(wParam)) ;
   if (LOWORD(wParam) != SB_ENDSCROLL)
      return false;
   if (hwndPixDiamSpin == (HWND) lParam) {
      test_init.diameter = read_count(hwndPixDiam) ;
      test_led->set_dimens(test_init.diameter, test_init.bit_gap, test_init.char_gap) ;
      update_display() ;
      return true;
   } 
   if (hwndBitGapSpin == (HWND) lParam) {
      test_init.bit_gap = read_count(hwndBitGap) ;
      test_led->set_dimens(test_init.diameter, test_init.bit_gap, test_init.char_gap) ;
      update_display() ;
      return true;
   } 
   if (hwndCharGapSpin == (HWND) lParam) {
      test_init.char_gap = read_count(hwndCharGap) ;
      test_led->set_dimens(test_init.diameter, test_init.bit_gap, test_init.char_gap) ;
      update_display() ;
      return true;
   } 
   return false ;
}

//*******************************************************************
//  handle minimize/restore
//*******************************************************************
static bool do_size(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LPVOID private_data)
{
   if (wParam == SIZE_RESTORED) {
      handle_restore = true ;
      main_timer_id = SetTimer(hwnd, IDT_TIMER_MAIN, 50, (TIMERPROC) NULL) ;
      //  I'm getting here on Restore, but update_display() is failing...
      //  Okay, this is actually working, but something else after this
      //  redraws the entire window, and erases this!!
      // update_display() ;
      // return true ;
   } 
   return false ;
}

//*******************************************************************
//  handle drag off-screen and the back on-screen,
//  or redraw after being covered by something else, then restored
//*******************************************************************
static bool do_erasebkgnd(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LPVOID private_data)
{
   if (init_done)
      update_display() ;
   return false ;
}

//*******************************************************************
static bool do_command(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LPVOID private_data)
{
   TCHAR msgstr[81] ;
   int result ;

   DWORD cmd = HIWORD (wParam) ;
   DWORD target = LOWORD(wParam) ;
   // termout(&this_term, "WM_COMMAND: cmd=%u, target=%u", cmd, target) ;
   if (cmd == BN_CLICKED) {
      switch (target) {
      case IDC_RB_SQUARE:
      case IDC_RB_ROUND:
         test_init.pixel_type = target - IDC_RB_ROUND ;
         CheckRadioButton (hwnd, IDC_RB_ROUND, IDC_RB_SQUARE, IDC_RB_ROUND + test_init.pixel_type) ;
         // show_statbar_msgf("pixel type = %s", (test_init.pixel_type == ROUND_PIXELS) ? "Round" : "Square") ;
         test_led->set_options(WHOLE_SPACE, test_init.pixel_type) ;
         update_display() ;
         return true;

      case IDB_LOAD_FONT:
         if (read_font_file(hwnd)) {
            wsprintf(msgstr, "%s", test_init.font_name) ;
            SetWindowText(hwndFontName, msgstr) ;
            update_display() ;
            return true;
         }
         return true;

      case IDB_ATTR_SET:
         result = select_color(hwnd, test_init.set_bit) ;
         if (result >= 0) {
            test_init.set_bit = (COLORREF) result ;
            fill_color_field(hwndSetAttr,   test_init.set_bit) ;
            test_led->set_attr(test_init.set_bit, test_init.clear_bit, test_init.bgnd) ;
            update_display() ;
         }
         return true;

      case IDB_ATTR_CLEAR:
         result = select_color(hwnd, test_init.clear_bit) ;
         if (result >= 0) {
            test_init.clear_bit = (COLORREF) result ;
            fill_color_field(hwndClearAttr, test_init.clear_bit) ;
            test_led->set_attr(test_init.set_bit, test_init.clear_bit, test_init.bgnd) ;
            update_display() ;
         }
         return true;

      case IDB_ATTR_BGND:
         result = select_color(hwnd, test_init.bgnd) ;
         if (result >= 0) {
            test_init.bgnd = (COLORREF) result ;
            fill_color_field(hwndBgndAttr,  test_init.bgnd) ;
            test_led->set_attr(test_init.set_bit, test_init.clear_bit, test_init.bgnd) ;
            update_display() ;
         }
         return true;
         
      case IDOK:
         PostMessage(hwnd, WM_CLOSE, 0, 0);
         return true;

      default:
         break;
      }  //lint !e744
   } 
   return false ;
}

//*******************************************************************
//  handle main-dialog resizing
//*******************************************************************
static bool do_sizing(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LPVOID private_data)
{
   switch (wParam) {
   case WMSZ_BOTTOMLEFT:
   case WMSZ_BOTTOMRIGHT:
   case WMSZ_TOPLEFT:
   case WMSZ_TOPRIGHT:
   case WMSZ_LEFT:
   case WMSZ_RIGHT:
   case WMSZ_TOP:
   case WMSZ_BOTTOM:
      // resize_child_dialog(true);
      return true;

   default:
      return false;
   }
}

//*******************************************************************
#define  SF_MIN_DX      950
#define  SF_MIN_DY      400

static bool do_getminmaxinfo(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LPVOID private_data)
{
   LPMINMAXINFO lpTemp = (LPMINMAXINFO) lParam;
   POINT        ptTemp;
   //  set minimum dimensions
   ptTemp.x = SF_MIN_DX ;  //  empirical value
   ptTemp.y = SF_MIN_DY ;  //  empirical value
   lpTemp->ptMinTrackSize = ptTemp;
   //  set maximum dimensions
   ptTemp.x = get_screen_width() ;
   ptTemp.y = get_screen_height() ;
   lpTemp->ptMaxTrackSize = ptTemp;
   return true ;
}

//*******************************************************************
static bool do_exitsizemove(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LPVOID private_data)
{
   // save_window_origin() ;  //  update config file if necessary
   update_display() ;
   return true ;
}

//*******************************************************************
static bool do_close(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LPVOID private_data)
{
   DestroyWindow(hwnd);
   return true ;
}

//*******************************************************************
static bool do_destroy(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LPVOID private_data)
{
   delete MainStatusBar ;
   MainStatusBar = NULL ;

   delete test_led ;
   test_led = NULL ;

   init_done = false ;
   handle_restore = false ; 
   
   PostQuitMessage(0);  //  this closes parent as well
   return true ;
}

//*******************************************************************
// typedef struct winproc_table_s {
//    uint win_code ;
//    bool (*winproc_func)(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LPVOID private_data) ;
// } winproc_table_t ;

static winproc_table_t const winproc_table[] = {
{ WM_INITDIALOG,     do_init_dialog },
{ WM_COMMAND,        do_command },
{ WM_TIMER,          do_timer },
{ WM_VSCROLL,        do_vscroll },
{ WM_SIZE,           do_size },
{ WM_ERASEBKGND,     do_erasebkgnd },
{ WM_GETMINMAXINFO,  do_getminmaxinfo },
{ WM_SIZING,         do_sizing },
{ WM_EXITSIZEMOVE,   do_exitsizemove },
{ WM_CLOSE,          do_close },
{ WM_DESTROY,        do_destroy },
{ 0, NULL } } ;

//*******************************************************************
static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
#ifdef  USE_WINMSGS
   switch (message) {
   //  list messages to be ignored
   case WM_CTLCOLORBTN:
   case WM_CTLCOLORSTATIC:
   case WM_CTLCOLOREDIT:
   case WM_MOUSEMOVE:
   case WM_NCMOUSEMOVE:
   case WM_NCHITTEST:
   case WM_SETCURSOR:
   case WM_TIMER:
   case WM_NOTIFY:
   case WM_COMMAND:  //  prints its own msgs below
      break;
   default:
      syslog("TOP [%s]\n", lookup_winmsg_name(message)) ;
      break;
   }
#endif
   uint idx ;
   for (idx=0; winproc_table[idx].win_code != 0; idx++) {
      if (winproc_table[idx].win_code == message) {
         return (*winproc_table[idx].winproc_func)(hwnd, message, wParam, lParam, NULL) ;
      }
   }
   return false;
}  //lint !e715

//*********************************************************************
// static bool font_viewer_active = false ;
// 
// bool show_font_viewer(void)
// {
//    if (font_viewer_active)
//       return false ;
//    font_viewer_active = true ;
//    load_exec_filename() ;     //  get our executable name
// 
//    //  create the main application
//    // DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SF_DIALOG), hwndParent, (DLGPROC) WndProc);
//    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SF_DIALOG), NULL, (DLGPROC) WndProc);
//    font_viewer_active = false ;
//    return true;
// }  //lint !e715

//***********************************************************************
//lint -esym(1784, WinMain)
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
   {
   load_exec_filename() ;     //  get our executable name

   HWND hwnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_SF_DIALOG), NULL, (DLGPROC) WndProc) ;
   if (hwnd == NULL) {
      syslog("CreateDialog: %s\n", get_system_message()) ;
      return 0;
   }

   MSG Msg;
   while(GetMessage(&Msg, NULL,0,0)) {
      if(!IsDialogMessage(hwnd, &Msg)) {
          TranslateMessage(&Msg);
          DispatchMessage(&Msg);
      }
   }

   return (int) Msg.wParam ;
}  //lint !e715

