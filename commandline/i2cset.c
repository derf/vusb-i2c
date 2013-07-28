#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "i2c-util.h"

int main(int argc, char **argv)
{
	int i, address, got_ack;

	i2c_init();
	i2c_start();

	if (argc < 3) {
		fputs("Usage: i2cset <address> <data ...>", stderr);
		return 1;
	}

	address = atoi(argv[1]) << 1;

	got_ack = i2c_tx_byte(address);

	for (i = 2; i < argc; i++) {
		i2c_tx_byte(atoi(argv[i]));
	}

	i2c_stop();
	i2c_deinit();

	return got_ack ? 0 : 1;
}
