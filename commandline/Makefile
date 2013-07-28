# Use the following 3 lines on Unix (uncomment the framework on Mac OS X):
USBFLAGS = `libusb-config --cflags`
USBLIBS = `libusb-config --libs` #-framework CoreFoundation

CC       ?= gcc
CFLAGS    = $(USBFLAGS) -O2 -Wall -Wextra -pedantic
LIBS      = $(USBLIBS)

PREFIX ?= /usr/local
bin_dir = ${DESTDIR}${PREFIX}/bin

PROGRAMS = vusb-i2c

all: $(PROGRAMS)

.c.o:
	$(CC) $(CFLAGS) -c $<

i2c.o: i2c.c i2c-util.c i2c-util.h
i2c-util.o: i2c-util.c
i2cdetect.o: i2cdetect.c

vusb-i2c: i2c.o i2c-util.o
	$(CC) -o $@ $^ $(LIBS)

vusb-i2cdetect: i2cdetect.o i2c-util.o
	$(CC) -o $@ $^ $(LIBS)

clean:
	rm -f *.o $(PROGRAMS)

install:
	install -m 0755 $(PROGRAMS) $(bin_dir)

uninstall:
	rm -rf $(bin_dir)/vusb-i2c $(bin_dir)/vusb-i2cdetect

.PHONY: all strip clean install uninstall
