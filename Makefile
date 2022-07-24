VERSION = 0.8
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/man

TTYOBJ = unikey.o runes.o
EXTRAOBJ += realpath.o strlcpy.o strlcat.o
FMOBJ = fm.o spawn.o strverscmp.o curses.o $(TTYOBJ) $(EXTRAOBJ)
NOPENOBJ = nopen.o spawn.o
BIN = fm
MAN = fm.1 nopen.1

all: $(BIN)

fm: $(FMOBJ)
	$(CC) $(CFLAGS) -o $@ $(FMOBJ) $(LDFLAGS)

nopen: $(NOPENOBJ)
	$(CC) $(CFLAGS) -o $@ $(NOPENOBJ) $(LDFLAGS) $(NOPENLDLIBS)

fm.o: arg.h fm.h util.h
dprintf.o: util.h
nopen.o: arg.h nopenconf.h util.h
spawn.o: util.h
strlcat.o: util.h
strlcpy.o: util.h
strverscmp.o: util.h

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(BIN) $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	cp -f $(MAN) $(DESTDIR)$(MANPREFIX)/man1

uninstall:
	cd $(DESTDIR)$(PREFIX)/bin && rm -f $(BIN)
	cd $(DESTDIR)$(MANPREFIX)/man1 && rm -f $(MAN)

clean:
	rm -f $(BIN) *.o
