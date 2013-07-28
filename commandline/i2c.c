#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "i2c-util.h"

int main(int argc, char **argv)
{
	i2c_init();

	char line[8];
	signed char i;
	short int number;

	set_sda(1);
	set_scl(1);
	usleep(1000);
	set_sda(0);
	usleep(1000);
	set_scl(0);
	verify_sda_low();
	verify_scl_low();
	puts("ready");

	while (fgets(line, 8, stdin) != NULL) {
		if (sscanf(line, "%hi\n", &number) == 1) {
			if ((number >= 0) && (number <= 256)) {
				for (i = 7; i >= -1; i--) {
					if ((i < 0) || (number == 256) || (number & (1 << i))) {
						puts("sda ↑");
						set_sda(1);
						//verify_sda_high();
					} else {
						puts("sda ↓");
						set_sda(0);
						//verify_sda_low();
					}
					usleep(10);
					puts("scl ↑");
					set_scl(1);
					usleep(10);
					//verify_scl_high();
					if (i < 0) {
						if (get_status() & (1 << BIT_SDA))
							puts("> NAK");
						else
							puts("> ACK");
					}
					else if (number == 256) {
						if (get_status() & (1 << BIT_SDA))
							puts("1");
						else
							puts("0");
					}
					puts("scl ↓");
					set_scl(0);
					usleep(10);
					//verify_scl_low();
				}
			}
		}
		if (strcmp(line, "push\n") == 0) {
			set_scl(1);
			usleep(30000);
			set_sda(1);
			usleep(100000);
			set_sda(0);
			usleep(100000);
			set_scl(0);
		}
	}

	set_scl(1);
	usleep(10);
	verify_scl_high();
	set_sda(1);
	usleep(10);
	verify_sda_high();

	i2c_deinit();
	return 0;
}
