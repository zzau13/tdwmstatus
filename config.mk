NAME = tdwmstatus
VERSION = 2.75

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

# runtime file paths (auto-detected at compile time; can be overridden)
TEMP_CANDIDATES = /sys/class/thermal/thermal_zone0/temp
BATTERY_STATUS_CANDIDATES = \
	/sys/class/power_supply/BAT0/status \
	/sys/class/power_supply/BAT1/status
CAPACITY_CANDIDATES = \
	/sys/class/power_supply/BAT0/capacity \
	/sys/class/power_supply/BAT1/capacity

TEMP_PATH ?= $(or $(firstword $(wildcard $(TEMP_CANDIDATES))),/sys/class/thermal/thermal_zone0/temp)
BATTERY_STATUS_PATH ?= $(or $(firstword $(wildcard $(BATTERY_STATUS_CANDIDATES))),/sys/class/power_supply/BAT0/status)
CAPACITY_PATH ?= $(or $(firstword $(wildcard $(CAPACITY_CANDIDATES))),/sys/class/power_supply/BAT0/capacity)

X11INC = /usr/X11/include
X11LIB = /usr/lib

# includes and libs
INCS = -I. -I/usr/include -I${X11INC}
LIBS = -L/usr/lib -lc -L${X11LIB} -lX11 -lasound

# flags
CPPFLAGS = -DVERSION=\"${VERSION}\" \
	-DTEMP_FILE=\"${TEMP_PATH}\" \
	-DBATTERY_STATUS_FILE=\"${BATTERY_STATUS_PATH}\" \
	-DCAPACITY_FILE=\"${CAPACITY_PATH}\"
CFLAGS = -std=c99 -pedantic -Wall -Wextra -Werror -Wno-implicit-function-declaration -O0 ${INCS} ${CPPFLAGS}
#CFLAGS = -g -std=c99 -pedantic -Wall -Wextra -O0 ${INCS} ${CPPFLAGS}
#LDFLAGS = -g ${LIBS}
LDFLAGS = -s ${LIBS}

# compiler and linker
CC = gcc

