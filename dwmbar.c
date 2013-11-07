#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

char *get_clock(char *buffer) {
	time_t rawtime;
	struct tm *info;

	time(&rawtime);
	info = localtime(&rawtime);
	strftime(buffer, 80, "%a %d. %b %H:%M", info);

	return buffer;
}


int main()
{
	Display *dpy;
	Window rootwin;
	char status[256];
	char clock[80];

 	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "ERROR: could not open display\n");
		exit(1);
	}
	
	rootwin = RootWindow(dpy, DefaultScreen(dpy));

	while(1) {
		get_clock(clock);

		/* set status line */
		snprintf(status, sizeof(status), clock);

		XStoreName(dpy, rootwin, status);
		XFlush(dpy);
		sleep(1);
	}

	XCloseDisplay(dpy);

	return 0;
}
