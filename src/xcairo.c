
#include "squabble.h"

Display *dpy;
Window root, win;
Cursor invisible_cursor;
static int scr;
static Pixmap buf;
static GC gc;
static unsigned int ww,wh,ig;

void xcairo_init() {
	if ( !(dpy=XOpenDisplay(0x0)) ) die("Unable to open display");
	scr = DefaultScreen(dpy);
	root = DefaultRootWindow(dpy);
	gc = DefaultGC(dpy,scr);
	win = XCreateSimpleWindow(dpy,root,0,0,800,600,1,0,0);
	XClassHint *hint = XAllocClassHint();
	hint->res_name = "Squabble";
	hint->res_class = "Squabble";
	XSetClassHint(dpy,win,hint);
	XStoreName(dpy,win,"Squabble");
	XFree(hint);
	XColor color;
	buf = XCreatePixmap(dpy,root, DisplayWidth(dpy,scr),
			DisplayHeight(dpy,scr), DefaultDepth(dpy,scr));
	char cdat = 0;
	Pixmap curs = XCreateBitmapFromData(dpy,win,&cdat,1,1);
	invisible_cursor = XCreatePixmapCursor(dpy,curs,curs,&color,&color,0,0);
	XFreePixmap(dpy,curs);
	XSetWindowAttributes wa;
	wa.event_mask = ExposureMask | KeyPressMask | ButtonPressMask |
			StructureNotifyMask;
	XChangeWindowAttributes(dpy,win,CWEventMask,&wa);
	XMapWindow(dpy,win);
	XFlush(dpy);
}

void xcairo_free() {
	XFreePixmap(dpy,buf);
	XUnmapWindow(dpy,win);
	XFlush(dpy);
	XDestroyWindow(dpy,win);
	XCloseDisplay(dpy);
}

void xcairo_start() {
	Window r;
	XGetGeometry(dpy,win,&r,&ig,&ig,&ww,&wh,&ig,&ig);
	cairo_surface_t *t_buf = cairo_xlib_surface_create(dpy, buf,
			DefaultVisual(dpy,scr), ww, wh);
	c = cairo_create(t_buf);
	csx = ww/1250.0; csy = wh/950.0;
	cairo_scale(c,csx,csy);
	cairo_surface_destroy(t_buf);
	cairo_set_line_width(c,4);
	cairo_set_line_join(c,CAIRO_LINE_JOIN_ROUND);
	cairo_set_font_size(c,20);
	cairo_select_font_face(c,"sans-serif",CAIRO_FONT_SLANT_NORMAL,
			CAIRO_FONT_WEIGHT_BOLD);
}

void xcairo_end() {
	XCopyArea(dpy,buf,win,gc,0,0,ww,wh,0,0);
	XFlush(dpy);
	cairo_destroy(c);
}

