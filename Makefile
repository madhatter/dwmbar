PROG 		= dwmbar
CC 			= gcc
PREFIX 	   ?= /usr/local
BINPREFIX 	= ${PREFIX}/bin

LIBS 	= -liw -lX11
CFLAGS 	= -Os -pedantic -Wall -Wextra -Wno-format-zero-length

debug: CFLAGS += -O0 -g
debug: ${PROG}

${PROG}: ${PROG}.c
	@${CC} ${CFLAGS} ${LIBS} -o ${PROG} ${PROG}.c
	@strip ${PROG}


clean:
	rm -f ${PROG}

