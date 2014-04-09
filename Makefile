PROG 		= dwmbar
CC 			= gcc
PREFIX 	   ?= /usr/local
BINPREFIX 	= ${PREFIX}/bin

# Comment these if no MPD is available or wanted
MPDLIBS  	= -lmpdclient
MPDFLAGS 	= -DMPD

SPOTIFYLIBS = $(shell pkg-config --cflags --libs glib-2.0) $(shell pkg-config --cflags --libs gio-2.0)
SPOTIFYFLAGS= -DSPOTIFY

LIBS 		= -liw -lX11 ${MPDLIBS} ${SPOTIFYLIBS}
CPPFLAGS 	= ${MPDFLAGS} ${SPOTIFYFLAGS}
CFLAGS 		= -Os -pedantic -Wall -Wextra -Wno-format-zero-length ${CPPFLAGS}

debug: CFLAGS += -O0 -g
debug: ${PROG}

${PROG}: ${PROG}.c
	@${CC} ${CFLAGS} ${LIBS} -o ${PROG} ${PROG}.c
	@strip ${PROG}

install:
	install -Dm755 ${PROG} ${BINPREFIX}/${PROG}

uninstall:
	rm -f ${BINPREFIX}/${PROG}

clean:
	rm -f ${PROG}

