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
#ifdef SPOTIFY
#include <glib.h>
#include <gio/gio.h>
#endif
#include "dwmbar.h"

struct wireless_info *wifi_info;
int skfd;
#ifdef MPD
struct mpd_connection *mpd_conn = NULL;
#endif
#ifdef SPOTIFY
typedef struct DBus {
	GError *error;
	GDBusConnection *bus;
} DBus;

struct DBus *dbus;
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

char *get_fan_output(char *buffer) {
	int fan1_out, fan2_out;
	char fan1_path[50], fan2_path[50];
	FILE *bfile;

	strcpy(fan1_path, FAN_PATH);
	strcat(fan1_path, "fan1_output");

	strcpy(fan2_path, FAN_PATH);
	strcat(fan2_path, "fan2_output");

	bfile = fopen(fan1_path, "r");
	if (bfile != NULL) {
		fscanf(bfile, "%i", &fan1_out);
		fclose(bfile);
	}

	bfile = fopen(fan2_path, "r");
	if (bfile != NULL) {
		fscanf(bfile, "%i", &fan2_out);
		fclose(bfile);
	}

	sprintf(buffer, "%d/%d", fan1_out, fan2_out);

	return buffer;
}

int is_up(char *device) {
	char devicepath[40], state[5];
	FILE *dfile;

	sprintf(devicepath, "/sys/class/net/%s/operstate", device);
	dfile = fopen(devicepath, "r");

	if (dfile != NULL) {
		fscanf(dfile, "%s", state);
		fclose(dfile);
		if(strcmp(state, "up") == 0)
			return 1;
	}

	return 0;
}

char *get_network_status(char *buffer) {
	sprintf(buffer, OFFLINE_STRING);

	if (is_up(ETHERNET_DEVICE))
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
	if (bfile != NULL) {
		fscanf(bfile, "%i", &batt_now);
		fclose(bfile);
	}

	bfile = fopen(BATTERY_FULL, "r");
	if (bfile != NULL) {
		fscanf(bfile, "%i", &batt_full);
		fclose(bfile);
	}

	/* When my battery is fully loaded batt_now has the designed
	 * full capacity instead of the full possible load.
	 * So batt_percent would be higher than 100%.
	 */
	if (batt_now / (batt_full / 100) < 100)
		batt_percent = batt_now / (batt_full / 100);
	else
		batt_percent = 100;

	if (batt_percent < 16)
		sprintf(buffer, "\x03%d%%\x01", batt_percent);
	else
		sprintf(buffer, " %d%% ", batt_percent);

	return buffer;
}

char *get_multi_battery_status(char *buffer) {
	int batt_now, batt1_now, batt2_now;
	int batt_full, batt1_full, batt2_full;
	int batt_percent;
	FILE *bfile;

	bfile = fopen(BATTERY_NOW, "r");
	if (bfile != NULL) {
		fscanf(bfile, "%i", &batt1_now);
		fclose(bfile);
	}

	bfile = fopen(BATTERY_FULL, "r");
	if (bfile != NULL) {
		fscanf(bfile, "%i", &batt1_full);
		fclose(bfile);
	}

	bfile = fopen(BATTERY_NOW2, "r");
	if (bfile != NULL) {
		fscanf(bfile, "%i", &batt2_now);
		fclose(bfile);
	}

	bfile = fopen(BATTERY_FULL2, "r");
	if (bfile != NULL) {
		fscanf(bfile, "%i", &batt2_full);
		fclose(bfile);
	}

	batt_now = batt1_now + batt2_now;
	batt_full = batt1_full + batt2_full;

	/* When my battery is fully loaded batt_now has the designed
	 * full capacity instead of the full possible load.
	 * So batt_percent would be higher than 100%.
	 */
	if (batt_now / (batt_full / 100) < 100)
		batt_percent = batt_now / (batt_full / 100);
	else
		batt_percent = 100;

	if (batt_percent < 16)
		sprintf(buffer, "\x03%d%%\x01", batt_percent);
	else
		sprintf(buffer, " %d%% ", batt_percent);

	return buffer;
}

char* rtrim(char* string, char junk) {
	char* original = string + strlen(string);

	while (*--original == junk);
	*(original + 1) = '\0';
	return string;
}

char *get_dropbox_status(char *buffer) {
	FILE *fp;
	char output[80];
	char command[20];

	sprintf(command, "%s status", DROPBOX_CMD);

	/* Open the command for reading. */
	fp = popen(command, "r");
	if (fp == NULL) {
		printf("Failed to run command\n" );
		exit(1);
	}

	/* read all output, keep the last line */
	while (fgets(output, sizeof(output)-1, fp) != NULL) {
		continue;
	}
	sprintf(buffer, " %s ", rtrim(output, '\n'));
	pclose(fp);

	return buffer;
}

#ifdef MPD
char *get_mpd_info(char *buffer) {
	struct mpd_status *status = NULL;
	struct mpd_song *song = NULL;
	const char *title = NULL, *artist = NULL;

	mpd_conn = mpd_connection_new(NULL, 0, 30000);

	if(mpd_connection_get_error(mpd_conn) != MPD_ERROR_SUCCESS) {
		sprintf(buffer, "Not connected (%s)", mpd_connection_get_error_message(mpd_conn));
		return buffer;
	}

	mpd_command_list_begin(mpd_conn, true);
	mpd_send_status(mpd_conn);
	mpd_send_current_song(mpd_conn);
	mpd_command_list_end(mpd_conn);
	status = mpd_recv_status(mpd_conn);

	if ((status) && ((mpd_status_get_state(status) == MPD_STATE_PLAY) ||
				(mpd_status_get_state(status) == MPD_STATE_PAUSE))) {
		mpd_response_next(mpd_conn);
		song = mpd_recv_song(mpd_conn);
		title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
		artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);

		if (mpd_status_get_state(status) == MPD_STATE_PLAY)
			sprintf(buffer, "> %s - %s", artist, title);
		else if (mpd_status_get_state(status) == MPD_STATE_PAUSE)
			sprintf(buffer, "|| %s - %s", artist, title);

		mpd_song_free(song);
		mpd_status_free(status);
		mpd_response_finish(mpd_conn);
	}
	else
		sprintf(buffer, "Not playing.");

	mpd_connection_free(mpd_conn);
	return buffer;
}
#endif

#ifdef SPOTIFY
char *get_spotify_info(char *buffer) {
	GVariant *result, *props;
	gchar **artists = NULL, *artist = NULL, *title = NULL;
	
	dbus->error = NULL;
	dbus->bus = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &dbus->error);
	if (!dbus->bus) {
		g_warning("dwmbar: Failed to connect to session bus: %s\n", dbus->error->message);
		g_error_free(dbus->error);
		sprintf(buffer, "DBus connection failed.");
		return buffer;
	}

	result = g_dbus_connection_call_sync(dbus->bus, "org.mpris.MediaPlayer2.spotify", "/org/mpris/MediaPlayer2",
			"org.freedesktop.DBus.Properties", "Get", g_variant_new("(ss)", "org.mpris.MediaPlayer2.Player", "Metadata"),
			G_VARIANT_TYPE("(v)"), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &dbus->error);
	
	if (dbus->error) {
		g_warning("dwmbar: Failed to call Get: %s\n", dbus->error->message);
		g_error_free(dbus->error);
		sprintf(buffer, "Spotify is not running.");
		return buffer;
	}
	
	g_variant_get(result, "(v)", &props);
	g_variant_lookup(props, "xesam:artist", "^as", &artists);
	g_variant_lookup(props, "xesam:title", "s", &title);
    g_variant_unref(props);

	if (artists)
		artist = g_strjoinv(", ", artists);
	else
		artist = "Unknown Artist";
	
	if (!title)
	   title = "Unknown Song";
	
	sprintf(buffer, "%s – %s", artist, title);

	return buffer;
}
#endif

int main()
{
	Display *dpy;
	Window rootwin;
	char status[256], clock[20], pacman[6], network[30], battery[10], fans[15];
	int net_cycles = 60;
	char dropbox[80];
	int dropbox_cycles = 30;

#ifdef MPD
	char mpd[100];
	int mpd_cycles = 20;
#endif

#ifdef SPOTIFY
	char spotify[100];
	int spot_cycles = 20;
#endif

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "ERROR: could not open display\n");
		exit(1);
	}

	rootwin = RootWindow(dpy, DefaultScreen(dpy));

	wifi_info = malloc(sizeof(struct wireless_info));
	memset(wifi_info, 0, sizeof(struct wireless_info));
	skfd = iw_sockets_open();

#ifdef SPOTIFY
	dbus = malloc(sizeof(struct DBus));
	memset(dbus, 0, sizeof(struct DBus));
#endif

	while (1) {
		get_clock(clock);
		get_pacman_updates(pacman);
		get_fan_output(fans);
		if (++net_cycles > 60) {
			net_cycles = 0;
			get_network_status(network);
		}
		if (ENABLE_BATTERY)
			get_battery_status(battery);

		if (ENABLE_MULTI_BATT)
			get_multi_battery_status(battery);

		if (ENABLE_DROPBOX && ++dropbox_cycles > 30) {
			dropbox_cycles = 0;
			get_dropbox_status(dropbox);
		}

#ifdef MPD
		if (++mpd_cycles > 20) {
			mpd_cycles = 0;
			get_mpd_info(mpd);
		}
#endif
#ifdef SPOTIFY
		if (++spot_cycles > 20) {
			spot_cycles = 0;
			get_spotify_info(spotify);
			g_object_unref(dbus->bus);
		}
#endif

		/* set status line */
#ifdef MPD
		sprintf(status, "%s :: %s :: %s ", mpd, network, pacman);
#elif SPOTIFY
		sprintf(status, "%s :: %s :: %s ", spotify, network, pacman);
#else
		sprintf(status, "%s :: %s ", network, pacman);
#endif
		if (ENABLE_DROPBOX)
			sprintf(status +strlen(status), "::%s", dropbox);

		if (ENABLE_BATTERY)
			sprintf(status +strlen(status), "::%s", battery);

		if (ENABLE_FAN)
			sprintf(status +strlen(status), "::%s", fans);

		sprintf(status +strlen(status), ":: %s", clock);

		XStoreName(dpy, rootwin, status);
		XFlush(dpy);

		sleep(1);
	}

	XCloseDisplay(dpy);
	iw_sockets_close(skfd);

	return EXIT_SUCCESS;
}
