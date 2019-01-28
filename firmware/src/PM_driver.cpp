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

				pm1 = (buff[3]<<8) + buff[4];
				pm25 = (buff[5]<<8) + buff[6];
				pm10 = (buff[7]<<8) + buff[8];
				pn03 = (buff[15]<<8) + buff[16];
				pn05 = (buff[17]<<8) + buff[18];
				pn1 = (buff[19]<<8) + buff[20];
				pn25 = (buff[21]<<8) + buff[22];
				pn5 = (buff[23]<<8) + buff[24];
				pn10 = (buff[25]<<8) + buff[26];

				for (uint8_t i=0; i<valuesSize; i++) {
					if (i < 6) values[i] = buff[i+3];
					else values[i] = buff[i+9];
				}

				lastReading = millis();
				active = true;

				return true;
			}
		}
	} else {
		if (millis() - lastReading < 10000) active = false;
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
	if (!getReading()) return false;

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
