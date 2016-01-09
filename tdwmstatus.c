/*
 * Compile with:
 * gcc -Wall -Wextra -O2 dwm-bar.c -o dwm-bar -lasound -lX11 
 *
 * author : Juan Aguilar Santillana
 * version : 2.74-1
 * date : 16/06/2015
 */

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>

#include <X11/Xlib.h>
#include <alsa/asoundlib.h>
#include "tdwmstatus.h"

#define MB 1048576

int 
main(void)
{
	char *status, batt_stat, *loadavg, *time, *uptime;
	char *kernel = get_kernel();
	uint_fast16_t ram, disk, temp, batt_cap, volume, n = 0;
	unsigned int net_send = 0, net_rec = 0;

	/* Static var */
	static Display *dpy;

	if (!(dpy = XOpenDisplay(NULL))) {
		perror("tdwmstatus: error: XOpenDisplay: ");
		return EXIT_FAILURE;
	};

	while(!(sleep(PERIOD)))
	{

		time = get_time();
		loadavg = get_loadavg();

		if(( n % PERIOD_1 ) == 0)
		{
			ram = get_ram();
			temp = get_temp();
			get_net(net_send, net_rec);
		}

		if(( n % PERIOD_2 ) == 0)
			volume = get_volume();
			batt_cap = get_batt_cap();
			batt_stat = get_batt_stat();

		if(( n % PERIOD_MAX) == 0 )
		{
			disk = get_disk();
			uptime = get_uptime();
		}

		n++;
		n = n % PERIOD_MAX;

		status = smprintf(OUTPUT_FORMAT,
				loadavg, 
				get_cpu(), 
				ram,
				disk,
				temp,
				net_send,
				net_rec,
				batt_cap,
				batt_stat, 
				kernel,
				volume,
				uptime,
				time);

		free(time);
		free(loadavg);

		set_status(dpy, status);
		free(status);
	}

	XCloseDisplay(dpy);
	return EXIT_SUCCESS;
}

static uint_fast16_t 
get_cpu(void)
{
	static uint_fast16_t previous_total = 0, previous_idle = 0;
	uint_fast16_t x, diff_total, diff_idle, buff[7];
	uint_fast16_t total = 0;

	FILE *file = fopen(STAT_FILE, "r");
	if (file == NULL) 
	{
		perror("tdwmstatus: error: CPU: ");
		return 1;
	}

	fscanf(file, "%*s %lu %lu %lu %lu %lu %lu %lu", 
			&buff[0], &buff[1], &buff[2], &buff[3], &buff[4], &buff[5], &buff[6]);

	fclose(file);

	for (x = 0; x < 7; x++)
		total += buff[x];

	diff_total     = total - previous_total;
	diff_idle      = buff[3] - previous_idle;

	previous_total = total;
	previous_idle  = buff[3];

	return ((1000 * (diff_total - diff_idle) / diff_total + 1) / 10);
}

static uint_fast16_t 
get_ram(void)
{
	uint_fast16_t used = 0, total = 0;

	struct sysinfo mem;
	sysinfo(&mem);

	total = (uint_fast16_t) mem.totalram / MB;
	used = (uint_fast16_t) (mem.totalram - mem.freeram - mem.bufferram - 
			mem.sharedram) / MB;
	return ((used * 100) / total);
}

static uint_fast16_t 
get_disk(void)
{
	struct statvfs buf;
	if(statvfs(getenv("HOME"), &buf))
	{
		perror("tdwmstatus: error: DISK: ");
		return 1;
	}
	return (uint_fast16_t)( 100 - (( (double)buf.f_bfree / 
					(double)buf.f_blocks ) * 100 ));
}

static uint_fast16_t 
get_volume(void)
{

	long int vol, max, min;
	int mute =0;
	snd_mixer_t *handle;
	snd_mixer_elem_t *elem;
	snd_mixer_selem_id_t *s_elem;

	snd_mixer_open(&handle, 0);
	snd_mixer_attach(handle, "default");
	snd_mixer_selem_register(handle, NULL, NULL);
	snd_mixer_load(handle);
	snd_mixer_selem_id_malloc(&s_elem);
	snd_mixer_selem_id_set_name(s_elem, "Master");
	elem = snd_mixer_find_selem(handle, s_elem);

	if (elem == NULL)
	{
		snd_mixer_selem_id_free(s_elem);
		snd_mixer_close(handle);

		perror("tdwmstatus: error: ALSA: ");
		return 1;
	}


	snd_mixer_handle_events(handle);
	snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
	snd_mixer_selem_get_playback_volume(elem, 0, &vol);

	snd_mixer_selem_get_playback_switch(elem, 0, &mute);
	printf("%d\n", mute);

	snd_mixer_selem_id_set_name(s_elem, "Headphone");
	elem = snd_mixer_find_selem(handle, s_elem);
	snd_mixer_selem_get_playback_switch(elem, 0, &mute);
	printf("%d\n", mute);
	
	snd_mixer_selem_id_set_name(s_elem, "Speaker");
	elem = snd_mixer_find_selem(handle, s_elem);
	snd_mixer_selem_get_playback_switch(elem, 0, &mute);
	printf("%d\n", mute);

	snd_mixer_selem_id_free(s_elem);
	snd_mixer_close(handle);

	return (uint_fast16_t)(vol * 100) / max;
}

static uint_fast16_t 
get_temp(void)
{
	FILE *file;
	uint_fast32_t ret; 

	if (!(( file = fopen(TEMP_FILE, "rt")))) 
	{
		if (!(( file = fopen(TEMP_FILE1, "rt"))))
		{
			perror("tdwmstatus error: TEMP: ");
			return 1;
		}
	}

	fscanf(file, "%lu", &ret);
	fclose(file);

	return ((uint_fast16_t) (ret / 1000));
}

static uint_fast16_t 
get_batt_cap(void)
{
	uint_fast16_t capacity;
	FILE *file;

	if (( file = fopen(CAPACITY_FILE, "r")) == NULL )
	{
		return 100;
	}
	else
		fscanf(file, "%lu", &capacity);
	fclose(file);
	return capacity;

}

static char *
get_kernel(void)
{
	struct utsname kernel;
	uname(&kernel);
	return smprintf("%s", kernel.release);
}

static char
get_batt_stat(void)
{
	FILE *file;
	char st;

	if ((file = fopen(BATTERY_STATUS_FILE, "r")) == NULL )
		return 'U';

	st = fgetc(file);
	fclose(file);
	return st;
}

static char *
get_time(void)
{
	char buff[32];
	time_t t = time(NULL);
	strftime(buff, sizeof(buff)-1, TIME_FORMAT, localtime(&t));
	return smprintf("%s", buff);
}

static char *
get_loadavg(void)
{
	double avgs[3];

	if (getloadavg(avgs, 3) < 0) {
		perror("tdwmstatus: error: getloadavg: ");
		exit(EXIT_FAILURE);
	}

	return smprintf("%.2f %.2f %.2f", avgs[0], avgs[1], avgs[2]);
}

static char *
get_uptime(void) 
{
	struct sysinfo info;
	int h,m = 0;
	sysinfo(&info);
	h = info.uptime/3600;
	m = (info.uptime - h*3600 )/60;
	return smprintf("%dh:%dm",h,m);
}

static void
get_net(unsigned int receivedabs, unsigned int sentabs)
{
	char buf[255];
	char *datastart;
	static int bufsize = 255;
	FILE *devfd;
	unsigned long long int receivedacc, sentacc;

	receivedabs = 0;
	sentabs = 0;

	if(!(devfd = fopen(NET_FILE, "r")))
	{
		perror("tdwmstatus: error: NET: ");
		return;
	}

	// Ignore the first two lines of the file
	fgets(buf, bufsize, devfd);
	fgets(buf, bufsize, devfd);

	while (fgets(buf, bufsize, devfd)) {
	    if ((datastart = strstr(buf, "lo:")) == NULL) {
		datastart = strstr(buf, ":");

		// With thanks to the conky project at http://conky.sourceforge.net/
		sscanf(datastart + 1, "%llu  %*d     %*d  %*d  %*d  %*d   %*d        %*d       %llu",\
		       &receivedacc, &sentacc);
		receivedabs += (unsigned int)receivedacc;
		sentabs += (unsigned int)sentacc;
	    }
	}

	free(datastart);
	fclose(devfd);

	if(receivedabs != 0)
		receivedabs /= 1024;
	if(sentabs !=0)
		sentabs /= 1024;
}

char *
smprintf(char *fmt, ...)
{
	va_list fmtargs;
	char *ret;
	int len;

	va_start(fmtargs, fmt);
	len = vsnprintf(NULL, 0, fmt, fmtargs);
	va_end(fmtargs);

	ret = malloc(++len);
	if (ret == NULL) {
		perror("tdwmstatus: error: malloc: ");
		exit(1);
	}

	va_start(fmtargs, fmt);
	vsnprintf(ret, len, fmt, fmtargs);
	va_end(fmtargs);

	return ret;
}

static void 
set_status(Display *dpy, char *str)
{
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}
