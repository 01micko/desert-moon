CC?=cc
PREFIX?=/usr/local
CFLAGS+=-Wall -pedantic -std=gnu99 $(shell pkg-config --cflags pangocairo)
LDFLAGS+=$(shell pkg-config --libs pangocairo)

all: moonwall

conf: conf.o
	$(CC) -o $@ $^ $(LDFLAGS)

conf.o: conf.c
	$(CC) -o $@ $(CFLAGS) -c $^

moonwall: conf.o moonwall.o
	$(CC) -o $@ $^ $(LDFLAGS)

moonwall.o: moonwall.c
	$(CC) -o $@ $(CFLAGS) -c $^

install:
	install -d -m 0755 $(DESTDIR)$(PREFIX)/bin
	install -s -m 0755 moonwall $(DESTDIR)$(PREFIX)/bin
	install -d -m 0755 $(DESTDIR)/etc
	install -m 0644 loc.conf $(DESTDIR)/etc

clean:
	-rm -f *.o moonwall
