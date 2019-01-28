#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <pins.h>

// Libraries for DallasTemp
#include <OneWire.h>
#include <DallasTemperature.h>


enum PMcommands {
	PM_START, 		// Start both PMS
	GET_PMA, 		// Get values for PMS in slot A
	GET_PMB, 		// Get values for PMS in slot A
	GET_PM_AVG, 		// Get values for both PMS averaged
	PM_STOP, 		// Stop both PMS
	DALLASTEMP_START,
	DALLASTEMP_STOP,
	GET_DALLASTEMP
};

static const uint8_t valuesSize = 18;

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

	uint32_t lastReading = 0;
	bool active = false;

	uint8_t values[valuesSize];

	// 24 bytes:
	// 0:1->pm1, 2:3->pm25, 4:5->pm10, 
	// 6:7->pm1, 8:9->pm2.5, 10:11->pm10   (under atmosferic enviroment)
	// Number of particles with diameter beyond X um in 0.1 L of air.
	// 12:13 -> 0.3 um
	// 14:15 -> 0.5 um
	// 16:17 -> 1.0 um
	// 18:19 -> 2.5 um
	// 20:21 -> 5.0 um
	// 22:23 -> 10.0 um

	// Readings
	uint16_t pm1;
	uint16_t pm25;
	uint16_t pm10;
	uint16_t pn03;
	uint16_t pn05;
	uint16_t pn1;
	uint16_t pn25;
	uint16_t pn5;
	uint16_t pn10;

private:

	HardwareSerial * _pmSerial;
	uint8_t _pinPOWER;
	uint8_t _pinENABLE;
	uint8_t _pinRESET;

	bool started = false;

	static const uint8_t buffLong = 27;
	unsigned char buff[buffLong];
};

class Sck_DallasTemp {
	public:
		bool start();
		bool stop();
		bool getReading();

		union u_reading {
			byte b[4];
			float fval;
		} uRead;

	private:
		uint8_t _oneWireAddress[8];
};
