#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
	Display *dpy;
	Window rootwin;
	char status[256];

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "ERROR: could not open display\n");
		exit(1);
	}
	
	rootwin = RootWindow(dpy, DefaultScreen(dpy));

	while(1) {
		snprintf(status, sizeof(status), "Test.");

		XStoreName(dpy, rootwin, status);
		XFlush(dpy);
		sleep(1);
	}

	printf("Done.\n");

	XCloseDisplay(dpy);

	return 0;
}
