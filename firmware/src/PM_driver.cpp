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

TinyGPSPlus gps;
TinyGPSCustom fixQuality(gps, "GPGGA", 6);

bool GrooveGps::start()
{
	// Check Serial port for GPS data 
	uint32_t start = millis();
	String sentence;

	while (millis() - start < 1000) {
		if (SerialGrove.available()) sentence += (char)SerialGrove.read();
		if (sentence.indexOf("$GP") >= 0) {
			enabled = true;
			return true;
		}
		if (sentence.length() > 4) sentence.remove(0, 1);
	}

	/*
	FIXME this command still doesn't work
	It will be good to avoid getting sentences we don't need
	0 NMEA_SEN_GLL,  // GPGLL interval - Geographic Position - Latitude longitude  
	1 NMEA_SEN_RMC,  // GPRMC interval - Recomended Minimum Specific GNSS Sentence 
	2 NMEA_SEN_VTG,  // GPVTG interval - Course Over Ground and Ground Speed  
	3 NMEA_SEN_GGA,  // GPGGA interval - GPS Fix Data  
	4 NMEA_SEN_GSA,  // GPGSA interval - GNSS DOPS and Active Satellites
	5 NMEA_SEN_GSV,  // GPGSV interval - GNSS Satellites in View
	17 NMEA_SEN_ZDA,  // GPZDA interval – Time & Date
	*/
	// Disable all except RMC and GGA
	/* sendCommand("PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"); */

	// TODO Change baudrate to 115200

  
	return false;
}

bool GrooveGps::stop()
{
	enabled = false;
	return true;
}

void GrooveGps::encode(char c)
{
	gps.encode(c);	
}

bool GrooveGps::getReading()
{
	// TODO decide how old is a reading still valid...
	
	
	// Valid -> uint8 - 1
	// 	0 = Invalid
	// 	1 = GPS fix (SPS)
	// 	2 = DGPS fix
	// 	3 = PPS fix
	// 	4 = Real Time Kinematic
	// 	5 = Float RTK
	// 	6 = estimated (dead reckoning) (2.3 feature)
	// 	7 = Manual input mode
	// 	8 = Simulation mode
	String fixQual = fixQuality.value(); 	// Convert ASCII digit to int (48 is ASCII '0')
	data[0] = fixQual.toInt();

	// Latitude DDD.DDDDDD (negative is south) -> double - 4
	data[1] = gps.location.lat();

	// Longitude DDD.DDDDDD (negative is west) -> double - 4
	data[5] = gps.location.lng();

	// Altitude in meters -> float - 4
	data[9] = gps.altitude.meters();

	// Time (epoch) -> uint32 - 4
	struct tm tm; 				// http://www.nongnu.org/avr-libc/user-manual/structtm.html
	tm.tm_isdst = -1; 			// -1 means no data available
	tm.tm_yday = 0;
	tm.tm_wday = 0;
	tm.tm_year = gps.date.year() - 1900; 	// tm struct expects years since 1900
	tm.tm_mon = gps.date.month() - 1; 	// tm struct uses 0-11 months
	tm.tm_mday = gps.date.day();
	tm.tm_hour = gps.time.hour();
	tm.tm_min = gps.time.minute();
	tm.tm_sec = gps.time.second();

	uint32_t epochTime = mktime(&tm);
	data[13] = epochTime;

	// Speed (meters per second) -> float - 4
	data[17] = gps.speed.mps();

	// Horizontal dilution of position -> float - 4
	data[21] = gps.hdop.value();

	// Number of Satellites being traked -> uint8 - 1
	data[25] = gps.satellites.value();


#ifdef debug_PM
	SerialUSB.print("Fix quality: ");
	SerialUSB.println(fixQual);
	SerialUSB.print("Latitude: ");
	SerialUSB.println(gps.location.lat(), 6);
	SerialUSB.print("Longitude: ");
	SerialUSB.println(gps.location.lng(), 6);
	SerialUSB.print("Altitude: ");
	SerialUSB.println(gps.altitude.meters());
	SerialUSB.print("Epoch time: ");
	SerialUSB.println(epochTime);
	SerialUSB.print("Speed (m per sec): ");
	SerialUSB.println(gps.speed.mps());
	SerialUSB.print("Hotizontal dilution: of precision: ");
	SerialUSB.println(gps.hdop.value());
	SerialUSB.print("Satellite number: ");
	SerialUSB.println(gps.satellites.value());
#endif

	return true;
}

uint16_t GrooveGps::getCheckSum(char* sentence)
{
	uint16_t checkSum = 0;

	for (uint16_t i=0; i<strlen(sentence); i++) {
		checkSum ^= sentence[i]; 
	}

	return checkSum;
}

bool GrooveGps::sendCommand(char* com)
{
	SerialUSB.print("Sending command: ");
	SerialUSB.println(com);

	uint8_t checkSum = getCheckSum(com);
	char checkSumSTR[3];
	sprintf(checkSumSTR, "*%02X", checkSum);
	
	for (uint16_t i=0; i<strlen(com); i++) {
		SerialGrove.write(com[i]);
		SerialUSB.write(com[i]);
	}

	SerialGrove.println(checkSumSTR);
	SerialUSB.println(checkSumSTR);
}


