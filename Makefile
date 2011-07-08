
VERSION = 0.23

CFLAGS += -O2 -g -Wall -W -D_FORTIFY_SOURCE=2 -fstack-protector \
	`pkg-config --cflags glib-2.0` \
	`pkg-config --libs glib-2.0`

OBJS := memuse.o proc.o report.o library.o

CC ?= gcc

all: memuse

memuse: $(OBJS) memuse.h
	@$(CC) -o memuse $(OBJS) $(CFLAGS)

%.o: %.c Makefile
	@[ -x /usr/bin/cppcheck ] && /usr/bin/cppcheck -q $< || :
	@$(CC) $(CFLAGS) -c -o $@ $<

install: memuse
	install -d $(DESTDIR)/usr/bin
	install -m 755 memuse $(DESTDIR)/usr/bin

clean:
	rm memuse -f *~ *.o

dist:
	git tag v$(VERSION)
	git archive --format=tar --prefix="memuse-$(VERSION)/" v$(VERSION) | \
		gzip > memuse-$(VERSION).tar.gz
