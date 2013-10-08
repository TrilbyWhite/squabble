
PROG     =  squabble
VER      =  0.1
CC       ?= gcc
CFLAGS   += `pkg-config --cflags x11 cairo`
LDFLAGS  += `pkg-config --libs x11 cairo` -pthread
PREFIX   ?= /usr
MODULES  =  squabble draw ai xcairo xevent tiles
HEADERS  =  squabble.h
OPTS		= -DSHARE_DIR=/usr/share/${PROG}/
VPATH		= src

${PROG}: ${MODULES:%=%.o}
	@cd src && ${CC} -o ../${PROG} ${MODULES:%=%.o} ${LDFLAGS}

%.o: %.c ${HEADERS}
	@${CC} -c -o src/$@ $< ${CFLAGS} ${OPTS}

install: ${PROG}
	@install -Dm755 ${PROG} ${DESTDIR}${PREFIX}/bin/${PROG}
	@install -Dm644 share/bonus ${DESTDIR}${PREFIX}/share/${PROG}/bonus
	@install -Dm644 share/dict ${DESTDIR}${PREFIX}/share/${PROG}/dict
	@install -Dm644 share/tiles ${DESTDIR}${PREFIX}/share/${PROG}/tiles

clean:
	@rm -f ${PROG} ${PROG}-${VER}.tar.gz
	@cd src && rm -f ${MODULES:%=%.o}

dist: clean
	@tar -czf ${PROG}-${VER}.tar.gz *

.PHONY: clean dist
