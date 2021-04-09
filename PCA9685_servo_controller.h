
#ifndef PCA9685_servo_controller_h
#define PCA9685_servo_controller_h

#include "hardware/i2c.h"
#include "pico/stdlib.h"

class PCA9685_servo_controller
{
public:
	// constructor, pass the I2C instance, the pins for SDA & SCL and the device I2C address
	PCA9685_servo_controller(i2c_inst_t *i2c, uint SDA, uint SCL, uint8_t i2c_address);	
	void begin(void);
	void writeRegister(uint8_t reg, uint8_t value);
	uint8_t readRegister(uint8_t reg);
	void setFrequency(uint16_t frequency);
	void setPosition(uint8_t channel, int8_t angle);			// -90 to +90
	void setPWM(uint8_t channel, int16_t PWM);
	long map(long x, long in_min, long in_max, long out_min, long out_max);
	
	void loop(uint64_t TEllapsed);
	
private:
	i2c_inst_t *_i2c;
	uint _SDA;
	uint _SCL;
	uint8_t _i2c_address;
};

#endif
