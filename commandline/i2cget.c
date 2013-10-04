#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "i2c-util.h"

int main(int argc, char **argv)
{
	int i, address, cmdbuf, got_ack;
	unsigned int ret;
	char *conv_err;

	i2c_init();
	i2c_start();

	if (argc < 3) {
		fputs("Usage: vusb-i2cget <address> <register ...> ", stderr);
		return 1;
	}

	address = strtol(argv[1], &conv_err, 0);

	if (conv_err && *conv_err) {
		fprintf(stderr, "address: Conversion error at '%s'", conv_err);
		return 2;
	}

	i2c_tx_byte((address << 1) | 0);

	for (i = 2; i < argc; i++) {
		cmdbuf = strtol(argv[i], &conv_err, 0);
		if (conv_err && *conv_err) {
			fprintf(stderr, "read command: Conversion error at '%s'", conv_err);
			return 2;
		}
		i2c_tx_byte(cmdbuf);
	}

	i2c_start();
	got_ack = i2c_tx_byte((address << 1) | 1);

	ret = i2c_rx_byte(0);

	i2c_stop();
	i2c_deinit();

	printf("%i\n", ret);

	return got_ack ? 0 : 1;
}
