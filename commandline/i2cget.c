#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "i2c-util.h"

int main(int argc, char **argv)
{
	int address, command;
	unsigned int ret;
	char mode = 'c';

	i2c_init();
	i2c_start();

	if (argc < 3) {
		fputs("Usage: vusb-i2cget <address> <register> [mode]", stderr);
		return 1;
	}

	address = atoi(argv[1]);
	command = atoi(argv[2]);

	if (argc == 4)
		mode = argv[3][0];

	i2c_tx_byte((address << 1) | 0);
	i2c_tx_byte(command);
	i2c_start();
	i2c_tx_byte((address << 1) | 1);

	if (mode == 'i') {
		ret = i2c_rx_byte(1);
		ret |= (i2c_rx_byte(0) << 8);
	}
	else
		ret = i2c_rx_byte(0);

	i2c_stop();
	i2c_deinit();

	printf("%i\n", ret);

	return 0;
}
