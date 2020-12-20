/*
 * title.c - window titles
 */

/*
 * Copyright 2006-2007 Johan Veenhuizen
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
#include <string.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>

#include "global.h"
#include "menu.h"
#include "lib.h"
#include "title.h"
#include "window.h"



static void prepare_repaint(struct widget *widget)
{
	struct title *title = (struct title *)widget;

	if (window_family_is_active(title->window)) {
		title->xftFg = fgColorTitleActive;
		title->xftBg = bgColorTitleActive;
	} else {
		title->xftFg = fgColorTitleInactive;
		title->xftBg = bgColorTitleInactive;
	}
}

static void repaint(struct widget *widget)
{
	struct title *title = (struct title *)widget;
	struct window *win = title->window;
	XftColor xftBg = title->xftBg;
	XftColor xftFg = title->xftFg;
	char *buf = NULL;
	int xpad = title_pad + 2 * xftfont->descent; /* this looks reasonable */
	int ypad = MAX(3, 2 * title_pad);
	int maxwidth = window_is_active(win) ?
	    WIDGET_WIDTH(title) - 2 * xpad - ypad - WIDGET_WIDTH(title) / 5 :
	    WIDGET_WIDTH(title) - 2 * xpad;
	int off;

	/* clear */
	XftDrawRect(title->xftdraw, &xftBg, 0, 0, WIDGET_WIDTH(title), WIDGET_HEIGHT(title));
	/* repaint */

	if (win->name != NULL && strlen(win->name) > 0) {
		buf = STRDUP(win->name);
		stringfit(buf, maxwidth);
		XftDrawString8(title->xftdraw, &xftFg, xftfont, xpad, title_pad+xftfont->ascent, (XftChar8 *) (buf), strlen(buf));
	}
	if (window_family_is_active(title->window)) {
		time_t t = time(NULL);
    		struct tm *tm = localtime(&t);
    		char s[64];
    		assert(strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S %A", tm));
		XftDrawString8(title->xftdraw, &xftFg, xftfont, WIDGET_WIDTH(title) - button_size - stringwidth(s), title_pad+xftfont->ascent, (XftChar8 *) (s), strlen(s));
	 }

	/* display */
	if (WIDGET_MAPPED(title))
		XCopyArea(display,
		    title->pixmap, WIDGET_XWINDOW(title), title->gc,
		    0, 0, WIDGET_WIDTH(title), WIDGET_HEIGHT(title), 0, 0);

	FREE(buf);
}

static void titleevent(struct widget *widget, XEvent *ep)
{
	struct title *title = (struct title *)widget;
	Window dummy;

	switch (ep->type) {
	case ButtonPress:
		if (ep->xbutton.button == Button1) {
			if (title->lastclick > ep->xbutton.time - 250) {
				maximize_window(title->window);
			} else if(!title->window->maximized){
				XTranslateCoordinates(display,
				    WIDGET_XWINDOW(title),
				    WIDGET_XWINDOW(title->window),
				    ep->xbutton.x,
				    ep->xbutton.y,
				    &title->xoff,
				    &title->yoff,
				    &dummy);
				if (ep->xbutton.state & ShiftMask)
					toggle_window_ontop(title->window);
				set_active_window(title->window);

				beginfastmove(WIDGET_XWINDOW(title));
				title->moving = 1;
			}
			title->lastclick = ep->xbutton.time;
		} else if (ep->xbutton.button == Button3)
			show_menu(winmenu,
			    ep->xbutton.x_root, ep->xbutton.y_root,
			    ep->xbutton.button);
		break;
	case ButtonRelease:
		if (ep->xbutton.button == Button1 && title->moving) {
			title->moving = 0;
			endfastmove();
		}
		break;
	case MotionNotify:
		if (title->moving) {
			if (ep->xmotion.state & ControlMask)
				move_window_family(title->window,
				    ep->xmotion.x_root - title->xoff,
				    ep->xmotion.y_root - title->yoff);
			else
				move_window(title->window,
				    ep->xmotion.x_root - title->xoff,
				    ep->xmotion.y_root - title->yoff);
		}
		break;
	case Expose:
		XCopyArea(display, title->pixmap, WIDGET_XWINDOW(title),
		    title->gc, ep->xexpose.x, ep->xexpose.y,
		    ep->xexpose.width, ep->xexpose.height,
		    ep->xexpose.x, ep->xexpose.y);
		break;
	}
}

struct title *create_title(struct window *window, int x, int y,
    int width, int height)
{
	XGCValues gcval;
	struct title *tp;

	tp = MALLOC(sizeof (struct title));
	create_widget(&tp->widget, WIDGET_TITLE, WIDGET_XWINDOW(window),
	    InputOutput, x, y, width, height, False);
	tp->pixmap = XCreatePixmap(display, WIDGET_XWINDOW(tp),
	    tp->pixmapwidth = width, tp->pixmapheight = height,
	    tp->widget.depth);
	tp->xwindow = tp->widget.xwindow;
	tp->xftdraw = XftDrawCreate(display, tp->pixmap, tp->widget.visual,tp->widget.colormap);
	gcval.graphics_exposures = False;
	tp->gc = XCreateGC(display, tp->pixmap,GCGraphicsExposures, &gcval);
	tp->window = window;
	tp->widget.event = titleevent;
	tp->widget.prepare_repaint = prepare_repaint;
	tp->widget.repaint = repaint;
	XSelectInput(display, WIDGET_XWINDOW(tp),
	    ButtonPressMask | ButtonMotionMask | ButtonReleaseMask |
	    ExposureMask);
	tp->moving = 0;
	tp->lastclick = 0;
	REPAINT(tp);
	map_widget(&tp->widget);
	return tp;
}

void resize_title(struct title *tp, int width, int height)
{
	if (width > tp->pixmapwidth || height > tp->pixmapheight) {
		XFreePixmap(display, tp->pixmap);
		if (width > tp->pixmapwidth)
			tp->pixmapwidth = MAX(LARGER(tp->pixmapwidth),
			                      width);
		if (height > tp->pixmapheight)
			tp->pixmapheight = MAX(LARGER(tp->pixmapheight),
			                       height);
		debug("increasing title pixmap size (%dx%d)",
		    tp->pixmapwidth, tp->pixmapheight);
		tp->pixmap = XCreatePixmap(display, WIDGET_XWINDOW(tp),
		    tp->pixmapwidth, tp->pixmapheight,
		    tp->widget.depth);
		XftDrawDestroy(tp->xftdraw);
		XFreeGC(display, tp->gc);
		XGCValues gcval;
		gcval.graphics_exposures = False;
		tp->gc =XCreateGC(display, tp->pixmap, GCGraphicsExposures, &gcval);
		tp->xftdraw = XftDrawCreate(display, tp->pixmap, tp->widget.visual,tp->widget.colormap);
	}

	resize_widget(&tp->widget, width, height);
	REPAINT(tp);
}

void destroy_title(struct title *title)
{
	destroy_widget(&title->widget);
	XftDrawDestroy(title->xftdraw);
	XFreePixmap(display, title->pixmap);
	XFreeGC(display, title->gc);
	FREE(title);
}
