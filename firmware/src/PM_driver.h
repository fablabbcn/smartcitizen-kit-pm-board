#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <pins.h>
#include <time.h>

// Libraries for DallasTemp
#include <OneWire.h>
#include <DallasTemperature.h>

// Library for GPS data parsing
#include "TinyGPS++.h"

#define debug_PM

enum PMcommands {
	PM_START, 		// Start both PMS
	GET_PMA, 		// Get values for PMS in slot A
	GET_PMB, 		// Get values for PMS in slot A
	GET_PM_AVG, 		// Get values for both PMS averaged
	PM_STOP, 		// Stop both PMS
	DALLASTEMP_START,
	DALLASTEMP_STOP,
	GET_DALLASTEMP,
	GROVEGPS_START,
	GROVEGPS_STOP,
	GROVEGPS_GET
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
	uint32_t lastFail = 0;
	bool active = false;

	uint8_t values[valuesSize];

	// Transmission via I2C to SCK in 18 bytes:
	// 0:1->pm1, 2:3->pm25, 4:5->pm10, 
	// Number of particles with diameter beyond X um in 0.1 L of air.
	// 6:7 -> 0.3 um
	// 8:9 -> 0.5 um
	// 10:11 -> 1.0 um
	// 12:13 -> 2.5 um
	// 14:15 -> 5.0 um
	// 16:17 -> 10.0 um

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

	static const uint8_t buffLong = 30; 	// Excluding both start chars

	// Serial transmission from PMS
	// 0: Start char 1 0x42 (fixed)
	// 1: Start char 2 0x4d (fixed)
	// 2-3 : Frame length = 2x13 + 2 (data + parity)

	// 4-5: PM1.0 concentration (CF = 1, standard particles) Unit ug/m^3
	// 6-7: PM2.5 concentration (CF = 1, standard particulates) Unit ug/m^3
	// 8-9: PM10 concentration (CF = 1, standard particulate matter) Unit ug/m^3

	// 10-11: PM1.0 concentration (in the atmosphere) Unit ug/m^3
	// 12-13: PM2.5 concentration (in the atmosphere) Unit ug/m^3
	// 14-15: PM10 concentration (in the atmosphere) Unit ug/m^3

	// 16-17: Particles in 0.1 liter of air > 0.3um 
	// 18-19: Particles in 0.1 liter of air > 0.5um 
	// 20-21: Particles in 0.1 liter of air > 1.0um 
	// 22-23: Particles in 0.1 liter of air > 2.5um 
	// 24-25: Particles in 0.1 liter of air > 5.0um 
	// 26-27: Particles in 0.1 liter of air > 10um 

	// 28: Version number
	// 29: Error code

	// 30-31: Sum of each byte from start_1 ... error_code 
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

class GrooveGps {
	public:
		bool start();
		bool stop();
		void encode(char c);
		bool getReading();
		
		// Data (34 bytes)
		// Fix Quality -> uint8 - 1
		// 	1 = GPS fix (SPS)
		// 	2 = DGPS fix
		// 	3 = PPS fix
		// 	4 = Real Time Kinematic
		// 	5 = Float RTK
		// 	6 = estimated (dead reckoning) (2.3 feature)
		// 	7 = Manual input mode
		// 	8 = Simulation mode
		// Latitude DDD.DDDDDD (negative is south) -> double - 4
		// Longitude DDD.DDDDDD (negative is west) -> double - 4
		// Altitude in meters -> float - 4
		// Time (epoch) -> uint32 - 4
		// Speed (meters per second) -> float - 4
		// Horizontal dilution of position -> float - 4
		// Number of Satellites being traked -> uint8 - 1

		static const uint8_t DATA_LEN = 34;
		byte data[DATA_LEN];
		bool started = false;

	private:
		uint16_t getCheckSum(char* sentence);
		bool sendCommand(char* com);
};
