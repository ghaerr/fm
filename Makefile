VERSION = 0.8
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/man

NOICELDLIBS = -lcurses
NOPENLDLIBS =
NOICEOBJ = dprintf.o noice.o spawn.o strlcat.o strlcpy.o strverscmp.o
NOPENOBJ = nopen.o spawn.o
BIN = noice nopen
MAN = noice.1 nopen.1

all: $(BIN)

noice: $(NOICEOBJ)
	$(CC) $(CFLAGS) -o $@ $(NOICEOBJ) $(LDFLAGS) $(NOICELDLIBS)

nopen: $(NOPENOBJ)
	$(CC) $(CFLAGS) -o $@ $(NOPENOBJ) $(LDFLAGS) $(NOPENLDLIBS)

dprintf.o: util.h
noice.o: arg.h noiceconf.h util.h
nopen.o: arg.h nopenconf.h util.h
spawn.o: util.h
strlcat.o: util.h
strlcpy.o: util.h
strverscmp.o: util.h

noiceconf.h:
	cp noiceconf.def.h $@

nopenconf.h:
	cp nopenconf.def.h $@

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(BIN) $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	cp -f $(MAN) $(DESTDIR)$(MANPREFIX)/man1

uninstall:
	cd $(DESTDIR)$(PREFIX)/bin && rm -f $(BIN)
	cd $(DESTDIR)$(MANPREFIX)/man1 && rm -f $(MAN)

dist: clean
	mkdir -p noice-$(VERSION)
	cp `find . -maxdepth 1 -type f` noice-$(VERSION)
	tar -c noice-$(VERSION) | gzip > noice-$(VERSION).tar.gz

clean:
	rm -f $(BIN) $(NOICEOBJ) $(NOPENOBJ) noice-$(VERSION).tar.gz
	rm -rf noice-$(VERSION)
