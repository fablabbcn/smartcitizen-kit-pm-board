#include "PM_driver.h"

bool PMsensor::begin()
{

	if (started) return true;

	pinMode(_pinENABLE, OUTPUT);
	digitalWrite(_pinENABLE, HIGH); 			// HIGH: Enable PMS

	pinMode(_pinPOWER, OUTPUT);
	digitalWrite(_pinPOWER, HIGH); 		// HIGH: Turn on power to PMS

	pinMode(_pinRESET, OUTPUT);
	digitalWrite(_pinRESET, HIGH);		// pull this pin down to reset module

	started = true;

	return true;
}
bool PMsensor::stop()
{

	digitalWrite(_pinENABLE, LOW);
	digitalWrite(_pinPOWER, LOW);
	_pmSerial->end();

	started = false;

	return true;
}
void PMsensor::reset()
{

	digitalWrite(_pinRESET, LOW);
	delay(200);
	digitalWrite(_pinRESET, HIGH);
}
bool PMsensor::update()
{

	if (_pmSerial->available()) {
		if (_pmSerial->find(0x42)) {
			_pmSerial->readBytes(buff, buffLong);
			if (buff[0] == 0x4d) {

				values[0] = buff[3];
				values[1] = buff[4];
				values[2] = buff[5];
				values[3] = buff[6];
				values[4] = buff[7];
				values[5] = buff[8];

				pm1 = (buff[3]<<8) + buff[4];
				pm25 = (buff[5]<<8) + buff[6];
				pm10 = (buff[7]<<8) + buff[8];

				lastReading = millis();

				return true;
			}
		}
	}
	return false;
}


OneWire oneWire = OneWire(GPIO0);
DallasTemperature _dallasTemp = DallasTemperature(&oneWire);

bool Sck_DallasTemp::start()
{
	_dallasTemp.begin();
	
	// If no device is found return false
	_dallasTemp.getAddress(_oneWireAddress, 0);
	if (_dallasTemp.validAddress(_oneWireAddress) <= 0) return false;

	_dallasTemp.setResolution(12);
	_dallasTemp.setWaitForConversion(true);

	return true;
}

bool Sck_DallasTemp::stop()
{

	return true;
}

bool Sck_DallasTemp::getReading()
{
	_dallasTemp.requestTemperatures();
	uRead.fval = _dallasTemp.getTempC(_oneWireAddress);
	if (uRead.fval <= DEVICE_DISCONNECTED_C) return false;
	
	return true;
}
