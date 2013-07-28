#define BIT_SDA 6
#define BIT_SCL 7

void verify_sda_low();
void verify_sda_high();
void verify_scl_low();
void verify_scl_high();

void set_sda(char value);
void set_scl(char value);

unsigned char get_status();
unsigned char i2c_tx_byte(unsigned char byte);
unsigned char i2c_rx_byte();

void i2c_start();
void i2c_stop();
void i2c_init();
void i2c_deinit();
