#
# This Makefile is part of qmenu, and is Copyright © 2014 Andrea Colangelo.
#
# This file is free software, and is released under the terms of the WTFPL, as
# described here: http://www.wtfpl.net/txt/copying/

SHELL = /bin/bash

RELEASE = 5.0.2

CC = gcc
CFLAGS = -g -Wall
LIBS = -lncurses

TARGET = qmenu

PREFIX = /usr/local
BINDIR= $(PREFIX)/bin
DATAROOTDIR = $(PREFIX)/share
EXAMPLESDIR = $(DATAROOTDIR)/doc/$(TARGET)/examples
MANDIR = $(DATAROOTDIR)/man/man1

build:
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c $(LIBS)

install:
	install -d -m 0755 $(BINDIR)
	install -d -m 0755 $(DATADIR)
	install -d -m 0755 $(MANDIR)
	install -m 0755 $(TARGET) $(BINDIR)/
	install -m 0644 *.mnu $(EXAMPLESDIR)
	install -m 0644 $(TARGET).1  $(MANDIR)

clean:
	$(RM) $(TARGET)
	$(RM) -rf .release-tmp

release: clean
	mkdir -p .release-tmp/$(TARGET)-$(RELEASE)/
	cp -rv * .release-tmp/$(TARGET)-$(RELEASE)
	rm -rf .release-tmp/$(TARGET)-$(RELEASE)/debian
	rm -f .release-tmp/$(TARGET)-$(RELEASE)/qmenu.{cfg,frm,gif,hlp}
	rm -f .release-tmp/$(TARGET)-$(RELEASE)/README_it.md
	tar czf $(TARGET)-$(RELEASE).tar.gz -C .release-tmp $(TARGET)-$(RELEASE)
	rm -rf .release-tmp

.PHONY: build install clean release
