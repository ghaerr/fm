#ifndef __UNIKEY__H
#define __UNIKEY__H

/*
 * ANSI to Unicode keyboard mapping
 */

/* return keyname */
char *keyname(int k);

/* convert from ANSI sequence to unicode key value */
int ansi_to_unikey(char *buf, int n);

/* state machine conversion of UTF-8 byte sequence to Rune */
int stream_to_rune(unsigned int ch);

/* Reads single keystroke or control sequence from character device */
int readansi(int fd, char *p, int n);

#define bsr(x)  ((x)? (__builtin_clz(x) ^ ((sizeof(int) * 8) -1)): 0)
#if 0
int bsr(int n)
{
    if (n == 0) return 0;   /* avoid incorrect result of 31 returned! */
    return (__builtin_clz(n) ^ ((sizeof(int) * 8) - 1));
}
#endif

/* defines from Cosmopolitan - see copyright in unikey.c */
#define ThomPikeCont(x)     (0200 == (0300 & (x)))
#define ThomPikeByte(x)     ((x) & (((1 << ThomPikeMsb(x)) - 1) | 3))
#define ThomPikeLen(x)      (7 - ThomPikeMsb(x))
#define ThomPikeMsb(x)      ((255 & (x)) < 252 ? bsr(255 & ~(x)) : 1)
#define ThomPikeMerge(x, y) ((x) << 6 | (077 & (y)))

/* when adding key values, add to unikeyname[] in unikey.c also */
typedef enum {
    /* ASCII range */
    kBackSpace = 8,
    kTab = 9,
    kEscKey = 27,
    kDel = 127,

    /* Unicode private use range 0xF700 - 0xF8FF */
#define kKeyFirst   kUpArrow
    kUpArrow = 0xF700,
    kDownArrow,
    kLeftArrow,
    kRightArrow,
    
    kInsert,
    kDelete,
    kHome,
    kEnd,
    kPageUp,
    kPageDown,

    /* TODO: entries for Alt-kUpArrow */

    /* leave out keypad keys for now */

    kAltA = 0xF800,
    kAltB,
    kAltC,
    kAltD,
    kAltE,
    kAltF,
    kAltG,
    kAltH,
    kAltI,
    kAltJ,
    kAltK,
    kAltL,
    kAltM,
    kAltN,
    kAltO,
    kAltP,
    kAltQ,
    kAltR,
    kAltS,
    kAltT,
    kAltU,
    kAltV,
    kAltW,
    kAltX,
    kAltY,
    kAltZ,
    kAlt0,
    kAlt1,
    kAlt2,
    kAlt3,
    kAlt4,
    kAlt5,
    kAlt6,
    kAlt7,
    kAlt8,
    kAlt9,

    kF1 = 0xF830,
    kF2,
    kF3,
    kF4,
    kF5,
    kF6,
    kF7,
    kF8,
    kF9,
    kF10,
    kF11,
    kF12,

    kCtrlF1 = 0xF840,
    kCtrlF2,
    kCtrlF3,
    kCtrlF4,
    kCtrlF5,
    kCtrlF6,
    kCtrlF7,
    kCtrlF8,
    kCtrlF9,
    kCtrlF10,
    kCtrlF11,
    kCtrlF12,

    kAltF1 = 0xF850,
    kAltF2,
    kAltF3,
    kAltF4,
    kAltF5,
    kAltF6,
    kAltF7,
    kAltF8,
    kAltF9,
    kAltF10,
    kAltF11,
    kAltF12
} unikey;

struct unikeyname {
    unikey  key;
    char *  name;
};

extern struct unikeyname unikeynames[];

#if 0
typedef enum {
    kMouseBtnLeft   = 0x0001,
    kMouseBtnRight  = 0x0002,
    kMouseBtnMiddle = 0x0010,
    kMouseWheelUp   = 0x0020,
    kMouseWheelDdown= ox0040,
    kMouseMotion
#endif

/* TUI mouse buttons bits, compatible with Windows and Nano-X */
#define MOUSE_BUTTON_L      0x0001
#define MOUSE_BUTTON_R      0x0002
#define MOUSE_BUTTON_M      0x0010
#define MOUSE_WHEEL_UP      0x0020
#define MOUSE_WHEEL_DOWN    0x0040

/* additional mouse status bits */
#define MOUSE_MOTION        0x0080  /* will have LRM button ORed in, if any */
#define MOUSE_UP            0x0100  /* otherwise down/motion, will have LRM ORed in */
#define MOUSE_MOD_SHIFT     0x0200  /* ORed into all events */
#define MOUSE_MOD_CTRL      0x0400  /* ORed into all events */
#define MOUSE_MOD_ALT       0x0800  /* ORed into all events */

/* Convert and decode ANSI mouse sequence ESC [ < status;y;x {m|M} */
int ansi_to_mouse(char *buf, int n, int *x, int *y, int *status);
const char *str_ansi_mouse_event(int e);
const char *str_tui_mouse_event(int e);

/* Check and decode ANSI DSR (device status report) ESC [ rows; cols R */
int ansi_dsr(char *buf, int n, int *rows, int *cols);
#endif
