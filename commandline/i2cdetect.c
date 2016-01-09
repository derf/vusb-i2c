#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "i2c-util.h"


int main(int argc, char **argv)
{
	unsigned char id, i2cid;

	i2c_getopt(argc, argv);
	i2c_init();

	fputs("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f", stdout);
	for (id = 0; id < 128; id++) {

		i2cid = (id << 1) | 1;

		fflush(stdout);

		if ((id & 0x0f) == 0) {
			printf("\n%02x:", id);
		}


		i2c_hw_start();

		if (i2c_hw_tx_byte(i2cid))
			printf(" %02x", id);
		else
			fputs(" --", stdout);

	}

	fputs("\n", stdout);

	i2c_hw_stop();
	i2c_deinit();
	return 0;
}
