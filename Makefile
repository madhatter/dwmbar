PROG 		= dwmbar
CC 			= gcc
PREFIX 	   ?= /usr/local
BINPREFIX 	= ${PREFIX}/bin

# Comment these if no MPD is available or wanted
MPDLIBS  	= -lmpdclient
MPDFLAGS 	= -DMPD

LIBS 		= -liw -lX11 ${MPDLIBS}
CPPFLAGS 	= ${MPDFLAGS}
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

