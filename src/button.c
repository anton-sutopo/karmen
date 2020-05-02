/*
 * button.c - window buttons
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

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "button.h"
#include "global.h"
#include "lib.h"
#include "menu.h"
#include "window.h"

static void setcolors(struct button *bp)
{
	bp->xftFg = window_family_is_active(bp->window)  ?
	   fgColorTitleActive : fgColorTitleInactive;
	bp->xftBg = window_family_is_active(bp->window) ?
	   bgColorTitleActive : bgColorTitleInactive;
}


static XftColor getXftBgPixel(struct button *bp) {
	setcolors(bp);
	if(bp->hover && !bp->depressed) {
		return window_family_is_active(bp->window) ? bgColorTitleActiveBright : bgColorTitleInactiveBright;
	} else {
		return window_family_is_active(bp->window) ? bgColorTitleActive : bgColorTitleInactive;
	}
}
static void prepare_repaint(struct widget *widget)
{

}

static void repaint(struct widget *widget)
{
	struct button *bp = (struct button *)widget;
	XftColor  xftBgPixel;

	setcolors(bp);
	xftBgPixel = getXftBgPixel(bp);

	/* clear */
	XftDrawRect(bp->xftdraw,&xftBgPixel, 0, 0, WIDGET_WIDTH(bp), WIDGET_HEIGHT(bp));

	/* draw */
		int x, y;

		x = 0;  
		y = 0; 
		if (bp->depressed) {
			x++;
			y++;
		}
		if(bp->drawing != NULL)
		bp->drawing(bp->xftdraw,bp->xftFg,x,y, WIDGET_WIDTH(bp), WIDGET_WIDTH(bp));

	/* display */
	if (WIDGET_MAPPED(bp))
		XCopyArea(display,
		    bp->pixmap, WIDGET_XWINDOW(bp), bp->gc,
		    0, 0, WIDGET_WIDTH(bp), WIDGET_HEIGHT(bp), 0, 0);
}

static void buttonevent(struct widget *widget, XEvent *ep)
{
	struct button *bp = (struct button *)widget;
	int docall;

	switch (ep->type) {
	case ButtonPress:
		if (ep->xbutton.button != Button1) {
			bp->acting = 0;
			bp->depressed = 0;
			if (ep->xbutton.button == Button3)
				show_menu(winmenu,
				    ep->xbutton.x_root, ep->xbutton.y_root,
				    ep->xbutton.button);
		} else {
			bp->acting = 1;
			bp->depressed = 1;
		}
		REPAINT(bp);
		break;
	case ButtonRelease:
		docall = (bp->acting && bp->depressed
		    && ep->xbutton.button == Button1
		    && bp->handler != NULL);
		bp->depressed = 0;
		bp->acting = 0;
		REPAINT(bp);
		/* must call handler as the last thing, it might destroy us */
		if (docall)
			bp->handler(bp->window);
		break;
	case Expose:
		XCopyArea(display, bp->pixmap, WIDGET_XWINDOW(bp),
		    bp->gc, ep->xexpose.x, ep->xexpose.y,
		    ep->xexpose.width, ep->xexpose.height,
		    ep->xexpose.x, ep->xexpose.y);
		break;
	case EnterNotify:
		if (bp->acting)
			bp->depressed = 1;
		bp->hover = 1;
		REPAINT(bp);
		break;
	case LeaveNotify:
		if (bp->acting)
			bp->depressed = 0;
		bp->hover = 0;
		REPAINT(bp);
		break;
	}
}

struct button *create_button(struct window *window, int x, int y,
    int width, int height)
{
	XGCValues gcval;
	struct button *bp;

	bp = MALLOC(sizeof (struct button));
	create_widget(&bp->widget, WIDGET_BUTTON, WIDGET_XWINDOW(window),
	    InputOutput, x, y, width, height);

	bp->pixmap = XCreatePixmap(display, WIDGET_XWINDOW(bp),
	    WIDGET_WIDTH(bp), WIDGET_HEIGHT(bp),
	    bp->widget.depth);
	
	bp->xftdraw = XftDrawCreate(display, bp->pixmap, bp->widget.visual, bp->widget.colormap); 
	gcval.graphics_exposures = False;
	bp->gc = XCreateGC(display, WIDGET_XWINDOW(bp),
	    GCGraphicsExposures, &gcval);
	//bp->image = NULL;

	bp->window = window;
	bp->acting = 0;
	bp->depressed = 0;
	bp->hover = 0;
	bp->handler = NULL;
	bp->widget.event = buttonevent;
	XSelectInput(display, WIDGET_XWINDOW(bp),
	    ButtonPressMask | ButtonReleaseMask |
	    ExposureMask | EnterWindowMask | LeaveWindowMask);
	bp->widget.prepare_repaint = prepare_repaint;
	bp->widget.repaint = repaint;
	REPAINT(bp);
	map_widget(&bp->widget);
	return bp;
}

void move_button(struct button *bp, int x, int y)
{
	move_widget(&bp->widget, x, y);
}

void destroy_button(struct button *bp)
{
	XFreePixmap(display, bp->pixmap);
	XftDrawDestroy(bp->xftdraw);
	XFreeGC(display, bp->gc);
	destroy_widget(&bp->widget);
	FREE(bp);
}

void set_button_handler(struct button *bp, void (*handler)(struct window *))
{
	bp->handler = handler;
}
void set_draw_handler (struct button *bp, void (*drawing)(XftDraw *xftdraw, XftColor xftcolor, int x, int y, int width, int height))
{
	bp->drawing = drawing;
}
//void set_button_image(struct button *bp, IMAGE *image)
//{
//	bp->image = image;
//	REPAINT(bp);
//}
