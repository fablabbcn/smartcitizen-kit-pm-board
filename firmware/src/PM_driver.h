#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <pins.h>


enum PMcommands { START, GET_PMA, GET_PMB, STOP};

class PMsensor {
public:

	// Constructor 
	PMsensor(HardwareSerial *serial, uint8_t pinPOWER, uint8_t pinENABLE, uint8_t pinRESET) {
		_pmSerial = serial;
		_pinPOWER = pinPOWER;
		_pinENABLE = pinENABLE;
		_pinRESET = pinRESET;
	}
	
	bool begin();
	bool stop();
	void reset();
	bool update();

	uint8_t values[6];		// 6 bytes 0:1->pm1, 2:3->pm25, 4:5->pm10


private:
	// Readings
	uint16_t pm1;
	uint16_t pm25;
	uint16_t pm10;

	HardwareSerial * _pmSerial;
	uint8_t _pinPOWER;
	uint8_t _pinENABLE;
	uint8_t _pinRESET;

	static const uint8_t buffLong = 23;
	unsigned char buff[buffLong];
};