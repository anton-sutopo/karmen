/*
 * lib.c - various routines
 */

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

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "global.h"
#include "lib.h"
#include "widget.h"

void error(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	fprintf(stderr, "karmen: ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	fflush(stderr);
	va_end(ap);
}

void debug(const char *fmt, ...)
{
#if defined(DEBUG)
	va_list ap;

	va_start(ap, fmt);
	fprintf(stderr, "karmen: DEBUG: ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	fflush(stderr);
	va_end(ap);
#endif
}

void unmapDraw(XftDraw *xftdraw, XftColor color, int x, int y, int width, int height) {
	XGlyphInfo ext;
        XftTextExtentsUtf8(display, xftfont, "<", 1, &ext);
        int y1 = y + height-(height - ext.height)/2;
        int x1 = x + (width - ext.width)/2;
	XftDrawString8(xftdraw, &color,xftfont,x1,y1, (XftChar8 *)"<",1);
}

void closeDraw(XftDraw *xftdraw, XftColor color, int x, int y, int width, int height) {
	XGlyphInfo ext;
	XftTextExtentsUtf8(display, xftfont, "#", 1, &ext);
	int y1 = y + height - (height - ext.height)/2;
	int x1 = x + (width - ext.width)/2;
	XftDrawString8(xftdraw, &color,xftfont,x1,y1, (XftChar8 *) "#",1);
}



int stringwidth(const char *str)
{
         XGlyphInfo ch;
        int direction;
        int ascent;
        int descent;

        XftTextExtents8(display, xftfont, (XftChar8 *)(str), strlen(str), &ch);
        return ch.width;
}

char *stringfit(char *str, int width)
{
	int len;

	len = strlen(str);
	while (stringwidth(str) > width) {
		if (len < 3) {
			str[0] = '\0';
			break;
		}
		strcpy(str + len - 3, "...");
		len--;
	}
	return str;
}

void *MALLOC(size_t size)
{
	void *ptr;

	while ((ptr = malloc(size)) == NULL && size != 0)
		sleep(1);
	return ptr;
}

void *REALLOC(void *optr, size_t size)
{
	void *nptr;

	while ((nptr = realloc(optr, size)) == NULL && size != 0)
		sleep(1);
	return nptr;
}

void FREE(void *ptr)
{
	free(ptr);
}

char *STRDUP(const char *str)
{
	char *new;

	assert(str != NULL);
	new = MALLOC(strlen(str) + 1);
	return strcpy(new, str);
}

static Window fastmovewin = None;

void beginfastmove(Window xwin)
{
	XSetWindowAttributes attr;

	if (fastmovewin != None)
		return;

	attr.override_redirect = True;
	fastmovewin = XCreateWindow(display, root,
	    0, 0, DisplayWidth(display, screen),
	    DisplayHeight(display, screen), 0, CopyFromParent, InputOnly,
	    CopyFromParent, CWOverrideRedirect, &attr);
	XMapWindow(display, fastmovewin);
	XGrabPointer(display, xwin, False,
	    ButtonMotionMask | ButtonReleaseMask,
	    GrabModeAsync, GrabModeAsync,
	    fastmovewin, None, CurrentTime);
}

void endfastmove(void)
{
	if (fastmovewin == None)
		return;

	XUngrabPointer(display, CurrentTime);
	XDestroyWindow(display, fastmovewin);
	fastmovewin = None;
}

#define CASE(type)	case type: return #type
const char *eventname(int type)
{
	switch (type) {
	CASE(KeyPress);
	CASE(KeyRelease);
	CASE(ButtonPress);
	CASE(ButtonRelease);
	CASE(MotionNotify);
	CASE(EnterNotify);
	CASE(LeaveNotify);
	CASE(FocusIn);
	CASE(FocusOut);
	CASE(KeymapNotify);
	CASE(Expose);
	CASE(GraphicsExpose);
	CASE(NoExpose);
	CASE(VisibilityNotify);
	CASE(CreateNotify);
	CASE(DestroyNotify);
	CASE(UnmapNotify);
	CASE(MapNotify);
	CASE(MapRequest);
	CASE(ReparentNotify);
	CASE(ConfigureNotify);
	CASE(ConfigureRequest);
	CASE(GravityNotify);
	CASE(ResizeRequest);
	CASE(CirculateNotify);
	CASE(CirculateRequest);
	CASE(PropertyNotify);
	CASE(SelectionClear);
	CASE(SelectionRequest);
	CASE(SelectionNotify);
	CASE(ColormapNotify);
	CASE(ClientMessage);
	CASE(MappingNotify);
	default:
		return "INVALID EVENT";
	}
}
