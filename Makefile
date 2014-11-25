PROG 		= dwmbar
CC 			= gcc
PREFIX 	   ?= /usr/local
BINPREFIX 	= ${PREFIX}/bin

# Choose between mpd and spotify
# Comment these if no MPD is available or wanted
MPDLIBS  	= -lmpdclient
MPDFLAGS 	= -DMPD

# Comment these if no spotify is available or wanted
#SPOTIFYLIBS = $(shell pkg-config --cflags --libs glib-2.0) $(shell pkg-config --cflags --libs gio-2.0)
#SPOTIFYFLAGS= -DSPOTIFY

LIBS 		= -liw -lX11 ${MPDLIBS} ${SPOTIFYLIBS}
CPPFLAGS 	= ${MPDFLAGS} ${SPOTIFYFLAGS}
CFLAGS 		= -Os -pedantic -Wall -Wextra -Wno-format-zero-length ${CPPFLAGS}

debug: CFLAGS += -O0 -g
debug: build

${PROG}: ${PROG}.c
	build
	@strip ${PROG}

build:
	@${CC} ${CFLAGS} ${LIBS} -o ${PROG} ${PROG}.c

install:
	install -Dm755 ${PROG} ${BINPREFIX}/${PROG}

uninstall:
	rm -f ${BINPREFIX}/${PROG}

clean:
	rm -f ${PROG}

