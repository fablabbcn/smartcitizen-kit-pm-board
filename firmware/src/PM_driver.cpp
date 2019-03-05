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
	if (millis() - lastReading < 1000) return true; 	// PM sensor only delivers one reading per second
	if (millis() - lastFail < 1000) return false; 		// We need at least one second after las fail

	// Empty serial buffer
	while(_pmSerial->available()) _pmSerial->read();

	// Wait for new readings
	uint32_t startPoint = millis();
	while(_pmSerial->available() < (buffLong + 2)) {
		if (millis() - startPoint > 1500) {
			// Timeout
			lastFail = millis();
#ifdef debug_PM
			SerialUSB.println("Timeout waiting for data!!");
#endif

			// After 10 seconds declare the PM innactive
			if (millis() - lastReading < 10000) {
				active = false;
#ifdef debug_PM
				SerialUSB.println("No data received in 10 seconds setting PM as innactive");
#endif
			}
			return false;
		}
	}

	uint16_t sum = 0;

	// Search for start char 1
	byte sc1 = 0;
	startPoint = millis();
	while (sc1 != 0x42) {
		sc1 = _pmSerial->read();
		if (millis() - startPoint > 1500) return false;
	}
	sum += sc1;

	// Confirm we receive start char 2
	byte sc2 = 0;
	sc2 = _pmSerial->read();

	if (sc2 == 0x4d) {

		sum += sc2;

		unsigned char buff[buffLong];
		byte howMany =  _pmSerial->readBytes(buff, buffLong);

#ifdef debug_PM
		SerialUSB.print("Received buffer ");
		SerialUSB.print(howMany);
		SerialUSB.println(" bytes:");
		for (uint8_t i=0; i<buffLong; i++) {
			SerialUSB.print(i);
			SerialUSB.print(": ");
			SerialUSB.println(buff[i]);
		}
#endif

		// Is buffer complete?
		if (howMany < 30) {
#ifdef debug_PM
			SerialUSB.println("ERROR: Received to few chars!");
#endif
			return false;
		}

		// Checksum
		uint16_t checkSum = (buff[28]<<8) + buff[29];
		for(int i=0; i<(buffLong - 2); i++) sum += buff[i];
		if(sum != checkSum) {
#ifdef debug_PM
			SerialUSB.println("Checksum ERROR!");
			SerialUSB.println(sum);
			SerialUSB.println(checkSum);
#endif
			return false;
		}

		// Get the values
		pm1 = (buff[2]<<8) + buff[3];
		pm25 = (buff[4]<<8) + buff[5];
		pm10 = (buff[6]<<8) + buff[7];
		pn03 = (buff[14]<<8) + buff[15];
		pn05 = (buff[16]<<8) + buff[17];
		pn1 = (buff[18]<<8) + buff[19];
		pn25 = (buff[20]<<8) + buff[21];
		pn5 = (buff[22]<<8) + buff[23];
		pn10 = (buff[24]<<8) + buff[25];

		// Prepare the raw values for I2C retransmission
		for (uint8_t i=0; i<valuesSize; i++) {
			if (i < 6) values[i] = buff[i+2];
			else values[i] = buff[i+8];
		}

		lastReading = millis();
		lastFail = 0;
		active = true;

		return true;
	}
#ifdef debug_PM
	SerialUSB.println("Unknown PM ERROR! (probably bad start char)");
#endif
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
