OBJS = pie.o ellipse.o
LIBS = -lcairo -lpixman-1 -lm
CFLAGS = -Wall -Wpedantic -g -O0 -I/usr/include/cairo

all: pie libpie.so libpie-static.a

libpie-static.a: $(OBJS)
	$(AR) rcv libpie-static.a $(OBJS)

libpie.so: $(OBJS)
	$(CC) -o libpie.so -shared libpie-static.a $(LIBS)

pie: libpie-static.a main.o
	$(CC) -o pie main.o libpie-static.a $(LIBS)

clean:
	rm -f pie $(OBJS) libpie.so libpie-static.a main.o

install:
	@if test -z "$(DESTDIR)"; then \
		echo -ne "\n\e[01;31mUsage: make install DESTDIR=\"<path>\"\e[00m\n\n"; \
		false; \
	else \
		mkdir -p $(DESTDIR)/bin || true ; \
		mkdir -p $(DESTDIR)/include || true ; \
		mkdir -p $(DESTDIR)/lib || true ; \
		cp pie $(DESTDIR)/bin; \
		cp pie.h $(DESTDIR)/include; \
		cp libpie-static.a $(DESTDIR)/lib; \
		cp libpie.so $(DESTDIR)/lib; \
	fi
