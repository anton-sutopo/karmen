#if !defined(GLOBAL_H)
#define GLOBAL_H

/*
 * Copyright 2006 Johan Veenhuizen
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

#include "lib.h"

#define DEFAULT_FONT	"sans:pixelsize=12"

extern Display *display;
extern int screen;
extern Window root;
extern XftFont *xftfont;
extern XftColor bgColorTitleActive;
extern XftColor bgColorTitleActiveBright;
extern XftColor fgColorTitleActive;
extern XftColor bgColorTitleInactive;
extern XftColor bgColorTitleInactiveBright;
extern XftColor fgColorTitleInactive;
extern XftColor bgColorMenu;
extern XftColor fgColorMenu;
extern XftColor bgColorMenuSelection;
extern XftColor fgColorMenuSelection;

extern int border_width;
extern int button_size;
extern int title_pad;



extern struct window *active;

extern struct menu *winmenu;

#endif /* !defined(GLOBAL_H) */
