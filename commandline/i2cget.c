#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "i2c-util.h"

int main(int argc, char **argv)
{
	int i, address, cmdbuf;
	int num_bytes = 1;
	unsigned int ret;
	char *conv_err;

	i2c_getopt(argc, argv);

	if (argc < 3) {
		fputs("Usage: vusb-i2cget <address> <num_bytes> [register ...] ", stderr);
		return 1;
	}

	address = strtol(argv[1], &conv_err, 0);

	if (conv_err && *conv_err) {
		fprintf(stderr, "address: Conversion error at '%s'\n", conv_err);
		return 2;
	}

	num_bytes = strtol(argv[2], &conv_err, 0);

	if (conv_err && *conv_err) {
		fprintf(stderr, "num_bytes: Conversion error at '%s'\n", conv_err);
		return 2;
	}

	i2c_init();

	if (argc >= 3) {
		i2c_hw_start();

		if (!i2c_hw_tx_byte((address << 1) | 0)) {
			fprintf(stderr, "Received NAK from slave 0x%02x, aborting\n", address);
			i2c_hw_stop();
			i2c_deinit();
			return 3;
		}
	}

	for (i = 3; i < argc; i++) {
		cmdbuf = strtol(argv[i], &conv_err, 0);
		if (conv_err && *conv_err) {
			fprintf(stderr, "read command: Conversion error at '%s'\n", conv_err);
			i2c_hw_stop();
			i2c_deinit();
			return 2;
		}
		if (!i2c_hw_tx_byte(cmdbuf)) {
			fprintf(stderr, "Received NAK after byte %d (0x%02x)\n", i-1, cmdbuf);
			i2c_hw_stop();
			i2c_deinit();
			return 4;
		}
	}

	i2c_hw_start();
	if (!i2c_hw_tx_byte((address << 1) | 1)) {
		fprintf(stderr, "Received NAK after reSTART from slave 0x%02x, aborting\n", address);
		i2c_hw_stop();
		i2c_deinit();
		return 3;
	}

	for (i = 1; i <= num_bytes; i++) {
		printf("%i ", i2c_hw_rx_byte((i < num_bytes) * 1));
	}
	putc('\n', stdout);

	i2c_hw_stop();
	i2c_deinit();

	return 0;
}
