CFLAGS += -O2 -g -Wall -W -D_FORTIFY_SOURCE=2 -fstack-protector `pkg-config --cflags glib-2.0` `pkg-config --libs glib-2.0`

OBJS := memuse.o proc.o report.o library.o

all: memuse


memuse:	$(OBJS)
	gcc $(CFLAGS) $(OBJS) -o memuse


clean:
	rm memuse -f *~ *.o
	