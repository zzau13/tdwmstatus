/*
 * version : 2.74-2
 * date : 06/03/2018
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <inttypes.h>

#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>

#include <X11/Xlib.h>
#include <alsa/asoundlib.h>
#define MB 1048576

#define TIME_FORMAT "%a %d/%m/%Y  %X"
#define OUTPUT_FORMAT " %s C: %hu%% T: %huº B: %hu%%%c V: %hu%% | %s | %s "  
/*  loadavg, cpu, temp, batt_cap, batt_stat, kernel, volume, uptime, time */

/* Times */
#define PERIOD 1
#define PERIOD_1 5
#define PERIOD_2 30
#define PERIOD_MAX 60

/* Files */
#define TEMP_FILE "/sys/class/thermal/thermal_zone0/temp"
#define BATTERY_STATUS_FILE	"/sys/class/power_supply/BAT0/status"
#define CAPACITY_FILE	"/sys/class/power_supply/BAT0/capacity"
#define STAT_FILE "/proc/stat"

static uint_fast16_t get_cpu(void);
static uint_fast16_t get_volume(void);
static uint_fast16_t get_temp(void);
static uint_fast16_t get_batt_cap(void);
static char get_batt_stat(void);
static char * get_time(void);
static char * get_loadavg_(void);
static char * get_uptime(void);
char * smprintf(char *, ...);
static void set_status(Display *, char *);

int 
main(void)
{
	char *status, *time, *loadavg, *uptime = NULL;

	static char batt_stat;
	static uint_fast16_t temp, batt_cap, volume, n = 0;
	static Display *dpy;

	if (!(dpy = XOpenDisplay(NULL))) {
		perror("tdwmstatus: error: XOpenDisplay: ");
		return EXIT_FAILURE;
	};

	while(!(sleep(PERIOD)))
	{

		if(( n % PERIOD_1 ) == 0)
		{
			temp = get_temp();
		}

		if(( n % PERIOD_2 ) == 0)
		{
			volume = get_volume();
			batt_cap = get_batt_cap();
			batt_stat = get_batt_stat();
		}

		if((n = (n + 1) % PERIOD_MAX) == 1 )
		{
			if (uptime != NULL) {
				free(uptime);
			}
			uptime = get_uptime();
		}

		time = get_time();
		loadavg = get_loadavg_();

		status = smprintf(OUTPUT_FORMAT,
				loadavg,
				get_cpu(), 
				temp,
				batt_cap,
				batt_stat, 
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

	return (100 * (diff_total - diff_idle) / diff_total + 1);
}

static uint_fast16_t 
get_volume(void)
{

	long int vol, max, min;
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
		perror("tdwmstatus error: TEMP: ");
		return 1;
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


static char
get_batt_stat(void)
{
	FILE *file;
	char status;

	if ((file = fopen(BATTERY_STATUS_FILE, "r")) == NULL )
		return 'U';

	status = fgetc(file);
	fclose(file);

	return status;
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
get_loadavg_(void)
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
		perror("tdwmstatus: error: malloc: smprintf");
		va_end(fmtargs);
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
