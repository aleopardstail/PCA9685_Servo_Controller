
#include "PCA9685_servo_controller.h"

PCA9685_servo_controller::PCA9685_servo_controller(i2c_inst_t *i2c, uint SDA, uint SCL, uint8_t i2c_address)
{
	_i2c = i2c;
	_SDA = SDA;
	_SCL = SCL;
	_i2c_address = i2c_address;
	
	return;
}

// a function to call every loop iteration
void PCA9685_servo_controller::loop(uint64_t TEllapsed)
{
	return;
}

void PCA9685_servo_controller::begin(void)
{
	// initialise I2C
	i2c_init(_i2c, 100 * 1000);
	gpio_set_function(_SDA, GPIO_FUNC_I2C);
	gpio_set_function(_SCL, GPIO_FUNC_I2C);
	gpio_pull_up(_SDA);
	gpio_pull_up(_SCL);
	
	// configure the PCA9685 for driving servos
	writeRegister(0x00, 0b10100000);
	writeRegister(0x01, 0b00000100);
	setFrequency(50);
	
	// centre all Servos initially
	for (uint C = 0; C < 16; C++)
	{
		setPosition(C, 0);
	}
	return;
}

void PCA9685_servo_controller::writeRegister(uint8_t reg, uint8_t value)
{
	uint8_t D[2];
	
	D[0] = reg;
	D[1] = value;
	
	i2c_write_blocking(_i2c, _i2c_address, D, 2, false);	// write register then value
	
	return;
}

uint8_t PCA9685_servo_controller::readRegister(uint8_t reg)
{
	uint8_t D[1];
	
	D[0] = reg;
	
	i2c_write_blocking(_i2c, _i2c_address, D, 1, false);	// write register
	i2c_read_blocking(_i2c, _i2c_address, D, 1, false);		// read value
	
	return D[0];
}

void PCA9685_servo_controller::setFrequency(uint16_t Frequency)
{
	int preScalerVal = (25000000 / (4096 * Frequency)) - 1;
    if (preScalerVal > 255) preScalerVal = 255;
    if (preScalerVal < 3) preScalerVal = 3;

	//need to be in sleep mode to set the pre-scaler
	uint8_t M1 = readRegister(0x00);
	writeRegister(0x00, ((M1 & ~0b10000000) | 0b00010000));
	writeRegister(0xFE, (uint8_t)preScalerVal);

	// restart
	writeRegister(0x00, ((M1 & ~0b00010000) | 0b10000000));
	sleep_us(500);		// <-- sleep functions not ideal, better to have a flag that clears after a period of time (or checks
						// to see if it should be cleared when its value is checked)
	return;
}

// set a position based on a supplied angle
void PCA9685_servo_controller::setPosition(uint8_t channel, int8_t angle)
{
	// map the angle to a PWM value (-90 to +90) to the servo
	// PWM_min & PWM_max values
	uint16_t PWM_min = 104;
	uint16_t PWM_max = 508;
	
	uint16_t PWM = (uint16_t) map(angle, -90, 90, PWM_min, PWM_max);
	uint16_t ChannelOffset = channel * 10;		// adds 0-160 to the counter values
	
	
	uint8_t D[5];
	
	uint16_t ChannelOn = 0 + ChannelOffset;
	uint16_t ChannelOff = PWM + ChannelOffset;
	
	D[0] = 0x06 + (4 * channel);
	D[1] = (0x00FF & ChannelOn);
	D[2] = (0xFF00 & ChannelOn) >> 8;
	D[3] = (0x00FF & ChannelOff);
	D[4] = (0xFF00 & ChannelOff) >> 8;
	
	i2c_write_blocking(_i2c, _i2c_address, D, 5, false);
	
	return;
}

// set a position based on a supplied PWM value (assumes "on" is at zero, offset applied here)
void PCA9685_servo_controller::setPWM(uint8_t channel, int16_t PWM)
{
	uint8_t D[5];
	
	uint16_t ChannelOffset = channel * 10;		// adds 0-160 to the counter values

	uint16_t ChannelOn = 0 + ChannelOffset;
	uint16_t ChannelOff = PWM + ChannelOffset;
	
	D[0] = 0x06 + (4 * channel);
	D[1] = (0x00FF & ChannelOn);
	D[2] = (0xFF00 & ChannelOn) >> 8;
	D[3] = (0x00FF & ChannelOff);
	D[4] = (0xFF00 & ChannelOff) >> 8;
	
	i2c_write_blocking(_i2c, _i2c_address, D, 5, false);
	
	return;
}

// useful utility function
long PCA9685_servo_controller::map(long x, long in_min, long in_max, long out_min, long out_max)
{
	return (x - in_min) * (out_max - out_min)/(in_max - in_min) + out_min;
}
