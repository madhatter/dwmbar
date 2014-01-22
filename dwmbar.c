#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <iwlib.h>
#ifdef MPD
#include <mpd/client.h>
#endif
#include "dwmbar.h"

struct wireless_info *wifi_info;
int skfd;
#ifdef MPD
struct mpd_connection *mpd_conn = NULL;
#endif

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

	fscanf(p_command, "%5s", buffer);
	
	pclose(p_command);
	return buffer;
}

int is_up(char *device) {
	char devicepath[40], state[5];
	FILE *dfile;

	sprintf(devicepath, "/sys/class/net/%s/operstate", device);
	dfile = fopen(devicepath, "r");
	
	if(dfile != NULL) {
		fscanf(dfile, "%s", state);
		fclose(dfile);
		if(strcmp(state, "up") == 0)
			return 1;
	}

	return 0;
}

char *get_network_status(char *buffer) {
	sprintf(buffer, OFFLINE_STRING);

	if(is_up(ETHERNET_DEVICE))
		sprintf(buffer, ETHERNET_STRING);
	else if (ENABLE_WIRELESS && is_up(WIRELESS_DEVICE)) {
		if (iw_get_basic_config(skfd, WIRELESS_DEVICE, &(wifi_info->b)) > -1) {
			if (wifi_info->b.has_essid && wifi_info->b.essid_on)
				sprintf(buffer, wifi_info->b.essid);		
		}
	}
	return buffer;
}

char *get_battery_status(char *buffer) {
	int batt_now, batt_full, batt_percent;
	FILE *bfile;

	bfile = fopen(BATTERY_NOW, "r");
	if(bfile != NULL) {
		fscanf(bfile, "%i", &batt_now);
		fclose(bfile);
	}

	bfile = fopen(BATTERY_FULL, "r");
	if(bfile != NULL) {
		fscanf(bfile, "%i", &batt_full);
		fclose(bfile);
	}

	/* When my battery is fully loaded batt_now has the designed
	 * full capacity instead of the full possible load.
	 * So batt_percent would be higher than 100%. 
	 */
	if(batt_now / (batt_full / 100) < 100)
		batt_percent = batt_now / (batt_full / 100);
	else
		batt_percent = 100;

	if(batt_percent < 16)
		sprintf(buffer, "\x03%d%%\x01", batt_percent);
	else
		sprintf(buffer, " %d%% ", batt_percent);

	return buffer;
}

char *get_mpd_info(char *buffer) {

	return buffer;
}

int main()
{
	Display *dpy;
	Window rootwin;
	char status[256], clock[80], pacman[6], network[30], battery[10], mpd[100];

 	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "ERROR: could not open display\n");
		exit(1);
	}
	
	rootwin = RootWindow(dpy, DefaultScreen(dpy));

	wifi_info = malloc(sizeof(struct wireless_info));
	memset(wifi_info, 0, sizeof(struct wireless_info));
	skfd = iw_sockets_open();

#ifdef MPD
	mpd_conn = mpd_connection_new(NULL, 0, 30000);
#endif

	while(1) {
		get_clock(clock);
		get_pacman_updates(pacman);
		get_network_status(network);
		get_battery_status(battery);
#ifdef MPD
		get_mpd_info(mpd);
#endif

		/* set status line */
		sprintf(status, "%s :: %s ", network, pacman);
		if(ENABLE_BATTERY)
			sprintf(status +strlen(status), "::%s", battery);
		sprintf(status +strlen(status), ":: %s", clock);

		XStoreName(dpy, rootwin, status);
		XFlush(dpy);
		sleep(1);
	}

	XCloseDisplay(dpy);
	iw_sockets_close(skfd);

	return EXIT_SUCCESS;
}
