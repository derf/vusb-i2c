#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "i2c-util.h"

#define BIT_SDA 6
#define BIT_SCL 7


int main(int argc, char **argv)
{
	signed char i;
	unsigned char id, i2cid;

	i2c_init();

	fputs("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f", stdout);
	for (id = 0; id < 128; id++) {

		i2cid = (id << 1) | 1;

		fflush(stdout);

		if ((id & 0x0f) == 0) {
			printf("\n%02x:", id);
		}


		i2c_start();

		if (i2c_tx_byte(i2cid))
			printf(" %02x", id);
		else
			fputs(" --", stdout);

		i2c_stop();
	}

	fputs("\n", stdout);

	i2c_deinit();
	return 0;
}
