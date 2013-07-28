#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "i2c-util.h"

#define BIT_SDA 6
#define BIT_SCL 7


int main(int argc, char **argv)
{

	i2c_init();

	signed char i;
	unsigned char id, i2cid;

	fputs("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f", stdout);
	for (id = 0; id < 128; id++) {

		i2cid = (id << 1) | 1;

		fflush(stdout);

		if ((id & 0x0f) == 0) {
			printf("\n%02x:", id);
		}

		set_sda(1);
		set_scl(1);
		usleep(1000);
		set_sda(0);
		usleep(1000);
		set_scl(0);
		verify_sda_low();
		verify_scl_low();
		for (i = 7; i >= -1; i--) {
			if ((i < 0) || (i2cid & (1 << i))) {
				set_sda(1);
				//verify_sda_high();
			} else {
				set_sda(0);
				//verify_sda_low();
			}
			usleep(10);
			set_scl(1);
			usleep(10);
			//verify_scl_high();
			if (i < 0) {
				if (get_status() & (1 << BIT_SDA))
					fputs(" --", stdout);
				else
					printf(" %02x", id);
			}
			set_scl(0);
			usleep(10);
			//verify_scl_low();
		}

		set_scl(1);
		usleep(100);
		//verify_scl_high();
		set_sda(1);
		usleep(100);
		//verify_sda_high();
	}

	fputs("\n", stdout);

	i2c_deinit();
	return 0;
}
