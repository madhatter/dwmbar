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

char *get_pacman_updates(char *buffer) {
	FILE *file = popen("pacman -Qu --dbpath /home/madhatter/pacman | wc -l 2>&1", "r");

	/*fgets(buffer, 2, file);*/
	fscanf(file, "%5s", buffer);
	
	return buffer;
}

int main()
{
	Display *dpy;
	Window rootwin;
	char status[256], clock[80], pacman[6];

 	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "ERROR: could not open display\n");
		exit(1);
	}
	
	rootwin = RootWindow(dpy, DefaultScreen(dpy));

	while(1) {
		get_clock(clock);
		get_pacman_updates(pacman);

		/* set status line */
		snprintf(status, sizeof(status), "%s :: %s", pacman, clock);

		XStoreName(dpy, rootwin, status);
		XFlush(dpy);
		sleep(1);
	}

	XCloseDisplay(dpy);

	return 0;
}
