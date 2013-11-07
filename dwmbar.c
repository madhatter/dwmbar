#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
	Display *dpy;
	Window rootwin;
	Window win;
	XEvent e;
	GC gc;

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "ERROR: could not open display\n");
		exit(1);
	}
	
	rootwin = RootWindow(dpy, DefaultScreen(dpy));
	win = XCreateSimpleWindow(dpy, rootwin, 1, 1, 100, 50, 0, 
		BlackPixel(dpy, DefaultScreen(dpy)), WhitePixel(dpy, DefaultScreen(dpy)));
	gc = XCreateGC(dpy, win, 0, NULL);

	XSelectInput(dpy, win, ExposureMask|ButtonPressMask);
	XStoreName(dpy, win, "hello");
	XSetForeground(dpy, gc, BlackPixel(dpy, DefaultScreen(dpy)));

	XMapWindow(dpy, win);

	while(1) {
		XNextEvent(dpy, &e);
		if(e.type == Expose && e.xexpose.count<1)
			XDrawString(dpy, win, gc, 10, 10, "Hello World!", 12);
		else if(e.type == ButtonPress) break;
	}

	printf("Done.\n");

	XFreeGC(dpy, gc);
	XDestroyWindow(dpy,win);
	XCloseDisplay(dpy);

	return 0;
}
