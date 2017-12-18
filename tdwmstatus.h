/*
 * Compile with:
 * gcc -Wall -Wextra -O2 dwm-bar.c -o dwm-bar -lasound -lX11 
 *
 * author : Juan Aguilar Santillana
 * version : 2.75
 * date : 18/12/2017
 */

#define TIME_FORMAT "%a %d/%m/%Y  %X"
#define OUTPUT_FORMAT " %s C: %hu%% T: %huº B: %hu%%%c V: %hu%% | %s | %s "  
/*  loadavg, cpu, temp, batt_cap, batt_stat, kernel, volume, uptime, time */

/* Times */
#define PERIOD 1
#define PERIOD_1 5
#define PERIOD_2 30
#define PERIOD_MAX 60

/* Files */
#define TEMP_FILE	"/sys/class/hwmon/hwmon0/temp1_input"
#define BATTERY_STATUS_FILE	"/sys/class/power_supply/BAT0/status"
#define CAPACITY_FILE	"/sys/class/power_supply/BAT0/capacity"
#define STAT_FILE "/proc/stat"

/* Functions prototypes*/
static uint_fast16_t get_cpu(void);
static uint_fast16_t get_volume(void);
static uint_fast16_t get_temp(void);
static uint_fast16_t get_batt_cap(void);
static char get_batt_stat(void);
static char * get_time(void);
static char * get_loadavg(void);
static char * get_uptime(void);
char * smprintf(char *, ...);
static void set_status(Display *, char *);
