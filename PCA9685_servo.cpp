#include "PCA9685_servo.h"

PCA9685_servo::PCA9685_servo(PCA9685_servo_controller *controller, uint8_t channel, uint16_t PWMmin, uint16_t PWMmax)
{
	_controller = controller;
	_channel = channel;
	_PWMmin = PWMmin;
	_PWMmax = PWMmax;
	return;
}

// a routine to call every loop iteration
void PCA9685_servo::loop(uint64_t TEllapsed)
{
	switch (_mode)
	{
		case MODE_FAST:
		{
			if (_isMoving)
			{
				// has enough time ellasped to allow us to have moved into position?
				_Tcntr += TEllapsed;
				
				if (_Tcntr >= _TFastDuration)
				{
					// we have finished moving
					_isMoving = 0;
					if (onStopMove != NULL) onStopMove(0);
				}
			}
			
			break;
		}
		
		case MODE_TCONSTANT:
		case MODE_SCONSTANT:
		{
			if (_isMoving)
			{
				// has enough time ellasped that we are due to move again?
				_Tcntr += TEllapsed;
				if (_Tcntr >= _TConstantPeriod)
				{
					_Tcntr = 0;
					// we need to move by one degree
					if (_currentAngle < _targetAngle)
					{
						// increase angle
						int8_t _A = _currentAngle++;
						
						if (_A < _minAngle) _A = _minAngle;
						if (_A > _maxAngle) _A = _maxAngle;
						
						uint16_t PWM = (uint16_t) _controller->map(_A, -90, 90, _PWMmin, _PWMmax);
						
						_controller->setPWM(_channel, PWM);
					}
					else
					{
						// decrease angle
						int8_t _A = _currentAngle--;
						
						if (_A < _minAngle) _A = _minAngle;
						if (_A > _maxAngle) _A = _maxAngle;
						
						uint16_t PWM = (uint16_t) _controller->map(_A, -90, 90, _PWMmin, _PWMmax);
						
						_controller->setPWM(_channel, PWM);
					}
				}
				
				// have we finished moving?
				if (_currentAngle == _targetAngle)
				{
					_isMoving = 0;
					if (onStopMove != NULL) onStopMove(0);
				}
			}
			break;
		}
		
	}
	
	return;
}

void PCA9685_servo::setPosition(int8_t Angle)
{
	switch (_mode)
	{
		// move to the set point as quickly as possible
		case MODE_FAST:
		{
			int8_t _A = Angle;
			_currentAngle = Angle;
			
			if (_A < _minAngle) _A = _minAngle;
			if (_A > _maxAngle) _A = _maxAngle;
			
			uint16_t PWM = (uint16_t) _controller->map(_A, -90, 90, _PWMmin, _PWMmax);
			
			_controller->setPWM(_channel, PWM);
			_isMoving = 1;
			_Tcntr = 0;
			if (onStartMove != NULL) onStartMove(0);
			break;
		}
		
		// move to the set point gradually over a constant time, note the screen update impacts this significantly
		// by occupying the I2C bus
		case MODE_TCONSTANT:
		{
			_targetAngle = Angle;	// where we wish to end up
			uint64_t AngleDelta = abs(Angle - _currentAngle);		// how many degrees we need to move
			
			// how long between 1 degree movements?
			_TConstantPeriod = _TConstantDuration / AngleDelta;	
			_isMoving = 1;
			_Tcntr = 0;
			if (onStartMove != NULL) onStartMove(0);
			break;
		}
		
		// move to the set point gradually at a set angular velocity
		case MODE_SCONSTANT:
		{
			_targetAngle = Angle;	// where we wish to end up
			_TConstantPeriod = _SConstantPeriod;
			
			_isMoving = 1;
			_Tcntr = 0;
			if (onStartMove != NULL) onStartMove(0);
			break;
		}
	}

	return;
}

int8_t PCA9685_servo::getPosition(void)
{
	return _currentAngle;
}

void PCA9685_servo::setRange(int8_t minAngle, int8_t maxAngle)
{
	_minAngle = minAngle;
	_maxAngle = maxAngle;
	_midAngle = ((_maxAngle - _minAngle) / 2) + _minAngle;
	return;
}

void PCA9685_servo::setRange(int8_t minAngle, int8_t midAngle, int8_t maxAngle)
{
	_minAngle = minAngle;
	_maxAngle = maxAngle;
	_midAngle = midAngle;
	return;
}
uint8_t PCA9685_servo::getInvertMode(void)
{
	return _invert;
}

void PCA9685_servo::setInvertMode(uint8_t Invert)
{
	_invert = Invert;
}

void PCA9685_servo::throwServo(uint8_t Value)
{
	if (_invert)
	{
		// inverted operation
		if (Value == 0)
		{
			setPosition(_maxAngle);
		}
		else
		{
			setPosition(_minAngle);
		}
	}
	else
	{
		// normal operation
		if (Value == 0)
		{
			setPosition(_minAngle);
		}
		else
		{
			setPosition(_maxAngle);
		}
	}
	return;
}

int8_t PCA9685_servo::getMinAngle(void)
{
	return _minAngle;
}

int8_t PCA9685_servo::getMaxAngle(void)
{
	return _maxAngle;
}

int8_t PCA9685_servo::getMidAngle(void)
{
	return _midAngle;
}

uint8_t PCA9685_servo::getMode(void)
{
	return _mode;
}

uint16_t PCA9685_servo::getAddress(void)
{
	return _address;
}

uint8_t PCA9685_servo::isMoving(void)
{
	return _isMoving;
}

uint64_t PCA9685_servo::getTConstantDuration(void)
{
	return _TConstantDuration;
}

uint64_t PCA9685_servo::getSConstantPeriod(void)
{
	return _SConstantPeriod;
}

void PCA9685_servo::setMinAngle(int8_t Angle)
{
	if (_currentAngle < Angle)
	{
		_currentAngle = Angle;
		setPosition(_currentAngle);
	}
	
	_minAngle = Angle;
	return;
}

void PCA9685_servo::setMidAngle(int8_t Angle)
{
	if ((Angle >= _minAngle) && (Angle <= _maxAngle))
	{
		_midAngle = Angle;
	}
	
	return;
}

void PCA9685_servo::setMaxAngle(int8_t Angle)
{
	if (_currentAngle > Angle)
	{
		_currentAngle = Angle;
		setPosition(_currentAngle);
	}
	_maxAngle = Angle;
	return;
}

void PCA9685_servo::setMode(uint8_t Mode)
{
	_mode = Mode;
	return;
}

void PCA9685_servo::setTConstantDuration(uint64_t TConstantDuration)
{
	_TConstantDuration = TConstantDuration;
	return;
}

void PCA9685_servo::setSConstantPeriod(uint64_t SConstantPeriod)	
{
	_SConstantPeriod = SConstantPeriod;
	return;
}

void PCA9685_servo::setAddress(uint16_t Address)
{
	_address = Address;
	return;
}
