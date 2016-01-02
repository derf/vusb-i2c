#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "i2c-util.h"

int main(int argc, char **argv)
{
	int i, address, cmdbuf;
	char *conv_err;

	i2c_getopt(argc, argv);

	if (argc < 3) {
		fputs("Usage: i2cset <address> <data ...>", stderr);
		return 1;
	}

	address = strtol(argv[1], &conv_err, 0);

	if (conv_err && *conv_err) {
		fprintf(stderr, "Conversion error at '%s'\n", conv_err);
		return 2;
	}

	i2c_init();
	i2c_start();

	if (!i2c_tx_byte(address << 1)) {
		fprintf(stderr, "Received NAK from slave 0x%02x, aborting\n", address);
		i2c_stop();
		i2c_deinit();
		return 3;
	}

	for (i = 2; i < argc; i++) {
		cmdbuf = strtol(argv[i], &conv_err, 0);
		if (conv_err && *conv_err) {
			fprintf(stderr, "write command: conversion error at '%s'\n", conv_err);
			i2c_stop();
			i2c_deinit();
			return 2;
		}
		if (!i2c_tx_byte(cmdbuf)) {
			fprintf(stderr, "Received NAK after byte %d (0x%02x)\n", i-1, cmdbuf);
			i2c_stop();
			i2c_deinit();
			return 4;
		}
	}

	i2c_stop();
	i2c_deinit();

	return 0;
}
