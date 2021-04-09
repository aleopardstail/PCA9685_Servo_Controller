(excuse the formatting, I need to sort out how this works)

# PCA9685_Servo_Controller
PCA9685 expansion board driver and interface for the Raspberry Pi Pico

this code includes code for two object classes, as outlined below

PCA9685_servo_controller (.h & .cpp)
====================================

this provides a class "PCA9685_servo_controller" which an instance of can be used to control a 
PCA9685 PWM controller board. This code is intended to be used to control servos and can be used
directly to manage servos in a simpple way

Constructor is called thus:

PCA9685_servo_controller SC(i2c0|i2c1, SDA Pin, SCL Pin, I2C Address);

it is then managed via a call to .begin().

functions are provided to allow direct access to the PCA9685 memory registers:
.readRegister(register) and .writeRegister(register, value), however it is easier to use the
more high level functions thus.

.setFrequency(Frequency in Hz);   // 50Hz is suggested and works
.setPosition(channel, angle);     // channel is the output channel number 0-15, angle is the 
                                    desired angle in degrees -90 to +90
                                    
controlling servos in this way makes use of default PWM values for the angle and may not be entirely
accurate, it does however work

PCA9685_servo (.h & .cpp)
=========================

this provides a class "PCA9685_servo" which can be instanced to provide better control over an individual
servo, this requires an instance of the controller class to have been created previously.

Constructor is called thus:

PCA9685_servo S(*PCA9685_servo_controller, channel (0-15), PWMmin, PWMmax);

the latter two have default values of 104 and 508, these can be adjusted gradually to fine tune the servo
position at -90 and +90 degrees

there is no ".begin()" function as initialisation is provided by the controller object which is used to actually
move the servo.

there are two function callbacks defined which may optionally be assigned and will be called prior to the 
servo starting to move and when it has completed its movement.

currently three modes of operation are defined in the .h file

MODE_FAST (the default), there the serv will be instructed to immediately move to the new position, the
only delays being the time taken for the hardware to respond

MODE_TCONSTANT, here the servo will move to the set angle over a constant time period, the default 
being 2 seconds which can be changed in the header file or set on a servo by servo basis using the function
.setTConstantDuration(time, in microseconds), e.g. moving from +90 to -90 in 2 secondsm then from -90 to -80 
again in 2 seconds.

MODE_SCONSTANT, here the servo will move with a constant angular velocity, by default taking 0.1 second to
move each degree, again this can be set in the header file as a default or set with a function call
.setSConstantPeriod(time, in microseconds)

to make the servos work you must call the .loop(fEllapsedTime in microseconds) function in each iteration of
your programme loop - see the example below

there are also further functions:
.setPosition(angle);          // sets a position to move to -90 to +90
.getPosition();               // returns the current angle, this is updated as the servo moves
.setRange(minAngle, maxAngle);  // sets the two end points, and calcuates a mid point
.setRange(minAngle, midAngle, maxAngle);  //sets the two end points and allows specification of a mid point
.getMinAngle();       // returns the minimum angle
.getMidAngle();       // returns the mid angle
.getMaxAngle(;        // returns the maximum angle
.getMode();           // gets which f the three defined modes is active
.isMoving();          // returns 1 if the servo is in motion, 0 otherwise
.getTConstantDuration();  // gets the time duration, in microseconds for movement in MODE_TCONSTANT
.getSCoonstantPeriod();   // gets the time period, in microseconds, to move 1 degree in MODE_SCONSTANT
.setMinAngle(Angle);    // defines the minimum angle, .setPosition clips at this
.setMidAngle(Angle);    // defines the mid angle
.setMaxAngle(Angle);    // defines the maximum angle, .setPosition clips at this
.setMode(mode);         // which of the three operating modes is desired
.setAddress(Address);   // sets a user specific address which is passed to the call back functions but 
                            not used internally, e.g. DCC Accessory decoder address
.setInvertMode(mode);   // 0 = normal, 1 = inverted, used with the ".throwServo(value)" command to invert operation
.throwServo(value);     // set the servo to one end point (0) or the other (1) in a single command


Example
=======
#include "PCA9685_servo.controller.h"
#include "PCA9685_servo.h"
#include <pico/stdlib.h>
#include <stdlib.h>
#include <stdio.h>

int main(void);
void StartMoveHandler(uint16_t Address);	// Servo callback
void StopMoveHandler(uint16_t Address);		// Servo callback

PCA9685_servo_controller myController(i2c0, 12, 13, 0x40);
PCA9685_servo myServo[16] = 
{
PCA9685_servo(&myController, 0),
	PCA9685_servo(&myController, 1),
	PCA9685_servo(&myController, 2),
	PCA9685_servo(&myController, 3),
	PCA9685_servo(&myController, 4),
	PCA9685_servo(&myController, 5),
	PCA9685_servo(&myController, 6),
	PCA9685_servo(&myController, 7),
	PCA9685_servo(&myController, 8),
	PCA9685_servo(&myController, 9),
	PCA9685_servo(&myController, 10),
	PCA9685_servo(&myController, 11),
	PCA9685_servo(&myController, 12),
	PCA9685_servo(&myController, 13),
	PCA9685_servo(&myController, 14),
	PCA9685_servo(&myController, 15)
};    // define an array of servos, note this could extend to use multiple controller on the i2c bus
uint64_t TNow = 0;
uint64_t TPrevious = 0;
uint64_t TEllapsed = 0;

int main(void)
{
  myController.begin();
  
  for (uint8_t i = 0; i<16; i++)
	{
		myServo[i].setRange(-30,30);
		myServo[i].setPosition(myServo[i].getMidAngle()); // move to mid point
		myServo[i].onStartMove = StartMoveHandler;    // set optional callback
		myServo[i].onStopMove = StopMoveHandler;
		myServo[i].setAddress(i);
    myServo[i].setMode(MODE_TCONSTANT);
	}
  
  while(1)
  {
    TNow = time_us_64();			// time now in microseconds
	  TEllapsed = TNow - TPrevious;	// time, in microseconds, since the last loop
	  TPrevious = TNow;				// store this ready for the next loop
	
	  // loop through the servos calling their loop function so they can do their thing
	  for (uint8_t i = 0; i<16; i++)
	  {
		  myServo[i].loop(TEllapsed);
	  }
    
    // insert commands to move the servos when required here
    // e.g.
    // myServo[4].throwServo(0);
  }
  
return 0;
}

void StartMoveHandler(uint16_t Address)
{
  // called when a servo starts to move
	return;
}

void StopMoveHandler(uint16_t Address)
{
  // called when a servo stops moving
	return;
}
 
