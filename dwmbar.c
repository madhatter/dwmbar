#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int main()
{
	Display *dpy;
	Window rootwin;
	char status[256];

	time_t rawtime;
	struct tm *info;
	char clock[80];

 	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "ERROR: could not open display\n");
		exit(1);
	}
	
	rootwin = RootWindow(dpy, DefaultScreen(dpy));

	while(1) {
		time(&rawtime);
		info = localtime(&rawtime);
		strftime(clock, sizeof(clock), "%a %d. %b %H:%M", info);

		/* set status line */
		snprintf(status, sizeof(status), clock);

		XStoreName(dpy, rootwin, status);
		XFlush(dpy);
		sleep(1);
	}

	XCloseDisplay(dpy);

	return 0;
}
