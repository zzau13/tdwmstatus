NAME = tdwmstatus
VERSION = 2.75-beta-01


# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/X11/include
X11LIB = /usr/lib

# includes and libs
INCS = -I. -I/usr/include -I${X11INC}
LIBS = -L/usr/lib -lc -L${X11LIB} -lX11 -lasound

# flags
CPPFLAGS = -DVERSION=\"${VERSION}\"
CFLAGS = -std=c99 -pedantic -Wall -Wextra -O0 ${INCS} ${CPPFLAGS}
#CFLAGS = -g -std=c99 -pedantic -Wall -Wextra -O0 ${INCS} ${CPPFLAGS}
#LDFLAGS = -g ${LIBS}
LDFLAGS = -s ${LIBS}

# compiler and linker
CC = gcc

