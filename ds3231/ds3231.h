#ifndef __DS3231_H
#define __DS3231_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define addr 0x68

/*the first verison use i2c1(GP6,GP7)*/
/*the new vesion use i2c0(GP20,GP21)*/
#define I2C_PORT	i2c1
#define I2C_SCL		7	
#define I2C_SDA		6

int ds3231SetTime();
void ds3231ReadTime();

#endif
