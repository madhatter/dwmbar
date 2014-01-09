#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "dwmbar.h"

char *get_clock(char *buffer) {
	time_t rawtime;
	struct tm *info;

	time(&rawtime);
	info = localtime(&rawtime);
	strftime(buffer, 80, "%a %d. %b %H:%M", info);

	return buffer;
}

char *get_pacman_updates(char *buffer) {
	char db_path[80];
	FILE *p_command;

	snprintf(db_path, sizeof(db_path), "pacman -Qu --dbpath %s | wc -l 2>&1", PACMAN_DB_PATH);
	p_command = popen(db_path, "r");

	/*fgets(buffer, 2, file);*/
	fscanf(p_command, "%5s", buffer);
	
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
