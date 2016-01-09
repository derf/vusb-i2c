void verify_sda_low();
void verify_sda_high();
void verify_scl_low();
void verify_scl_high();

void set_sda(char value);
void set_scl(char value);

unsigned char get_status();
unsigned char i2c_tx_byte(unsigned char byte);
unsigned char i2c_rx_byte(unsigned char send_ack);

unsigned char i2c_hw_tx_byte(unsigned char byte);
unsigned char i2c_hw_rx_byte(unsigned char send_ack);

void i2c_getopt(int argc, char **argv);
void i2c_start();
void i2c_stop();
void i2c_init();
void i2c_deinit();

void i2c_hw_setbits();
void i2c_hw_start();
void i2c_hw_stop();
