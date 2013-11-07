#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
	Display *dpy;

	if (!(dpy=XOpenDisplay(NULL))) {
		fprintf(stderr, "ERROR: could not open display\n");
		exit(1);
	}

	printf("Done.\n");

	return 0;
}
