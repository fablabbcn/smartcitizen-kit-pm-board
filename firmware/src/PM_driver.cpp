#include "PM_driver.h";

bool PMsensor::begin() {

	pinMode(_pinENABLE, OUTPUT);
	digitalWrite(_pinENABLE, HIGH); 			// HIGH: Enable PMS

	pinMode(_pinPOWER, OUTPUT);
	digitalWrite(_pinPOWER, HIGH); 		// HIGH: Turn on power to PMS

	pinMode(_pinRESET, OUTPUT);
	digitalWrite(_pinRESET, HIGH);		// pull this pin down to reset module

	return true;
}
bool PMsensor::stop() {

	digitalWrite(_pinENABLE, LOW);
	digitalWrite(_pinPOWER, LOW);
	_pmSerial->end();

	return true;
}
void PMsensor::reset() {

	digitalWrite(_pinRESET, LOW);
	delay(200);
	digitalWrite(_pinRESET, HIGH);
}
bool PMsensor::update() {

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
