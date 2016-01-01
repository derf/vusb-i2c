# Use the following 3 lines on Unix (uncomment the framework on Mac OS X):
USBFLAGS = `libusb-config --cflags`
USBLIBS = `libusb-config --libs` #-framework CoreFoundation

CC       ?= gcc
CFLAGS    = $(USBFLAGS) -O2 -Wall -Wextra -pedantic
LIBS      = $(USBLIBS)

PREFIX ?= /usr/local
bin_dir = ${DESTDIR}${PREFIX}/bin
man_dir = ${DESTDIR}${PREFIX}/man/man1

PROGRAMS = vusb-i2cdetect vusb-i2cset vusb-i2cget

all: $(PROGRAMS)

.c.o:
	$(CC) $(CFLAGS) -c $<

i2c-util.o: i2c-util.c
i2cdetect.o: i2cdetect.c i2c-util.c i2c-util.h
i2cset.o: i2cset.c i2c-util.c i2c-util.h
i2cget.o: i2cget.c i2c-util.c i2c-util.h

vusb-i2cdetect: i2cdetect.o i2c-util.o
	$(CC) -o $@ $^ $(LIBS)

vusb-i2cset: i2cset.o i2c-util.o
	$(CC) -o $@ $^ $(LIBS)

vusb-i2cget: i2cget.o i2c-util.o
	$(CC) -o $@ $^ $(LIBS)

clean:
	rm -f *.o $(PROGRAMS)

install:
	install -m 4755 $(PROGRAMS) $(bin_dir)
	install -m 0644 man/vusb-i2cdetect.1 man/vusb-i2cget.1 man/vusb-i2cset.1 \
	  $(man_dir)

uninstall:
	rm -f $(bin_dir)/vusb-i2cdetect
	rm -f $(bin_dir)/vusb-i2cget $(bin_dir)/vusb-i2cset
	rm -f $(man_dir)/vusb-i2cdetect.1 $(man_dir)/vusb-i2cget.1 $(man_dir)vusb-i2cset.1

.PHONY: all strip clean install uninstall
