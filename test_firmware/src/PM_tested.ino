#include "wiring_private.h"
#include <Arduino.h>
#include <Wire.h>

#include <pins.h>

void ledCycle() {

	SerialUSB.print("Cycling led colors... ");

	uint8_t pins[] = { pinRED, pinGREEN, pinBLUE };
	const char *titles[] = { "red, ", "green, ", "blue... "};

	for (uint8_t led=0; led<3; led++) {

		SerialUSB.print(titles[led]);

		for(uint8_t i=255; i>0; i--) {
			analogWrite(pins[led], 255-i);
			delay(5);
		}
		pinMode(pins[led], OUTPUT);
		digitalWrite(pins[led], HIGH);
	}

	SerialUSB.println("finished");
}

void searchPM() {

	uint8_t LENG = 23;
	unsigned char buf[LENG];

	SerialUSB.println("Searching for PM sensors... ");

	// Led red
	pinMode(pinRED, OUTPUT);
	digitalWrite(pinRED, LOW);

	const uint8_t required = 3;

	uint8_t numA = 0;
	uint8_t numB = 0;

	bool A = false;
	bool B = false;


	// RED: No PM detected
	// YELLOW: Only one PM detected
	// GREEN: Both PM detected

	while(true) {

		if (SerialPMA_A.available() && numA < required) {

			if (SerialPMA_A.find(0x42)) {
				SerialPMA_A.readBytes(buf,LENG);
				if (buf[0] == 0x4d) numA++;
				SerialUSB.print("PM_A 1.0 -> ");
				SerialUSB.print((buf[3]<<8) + buf[4]);
				SerialUSB.print(", PM_A 2.5 -> ");
				SerialUSB.print((buf[5]<<8) + buf[6]);
				SerialUSB.print(", PM_A 10 -> ");
				SerialUSB.print((buf[7]<<8) + buf[8]);
				SerialUSB.println();
				if (numA == required) {
					SerialUSB.println("Founded PM in connector A");
					analogWrite(pinBLUE, 80);
					A = true;
					if (!B) SerialUSB.println("To complete the test please connect a PM sensor on connector B");
					else break;
				}
			}
		} else if (SerialPMA_B.available() && numB < required) {

			if (SerialPMA_B.find(0x42)) {
				SerialPMA_B.readBytes(buf,LENG);
				if (buf[0] == 0x4d) numB++;
				SerialUSB.print("PM_B 1.0 -> ");
				SerialUSB.print((buf[3]<<8) + buf[4]);
				SerialUSB.print(", PM_B 2.5 -> ");
				SerialUSB.print((buf[5]<<8) + buf[6]);
				SerialUSB.print(", PM_B 10 -> ");
				SerialUSB.print((buf[7]<<8) + buf[8]);
				SerialUSB.println();
				if (numB == required) {
					SerialUSB.println("Founded PM in connector B");
					analogWrite(pinBLUE, 80);
					B = true;
					if (!A) SerialUSB.println("To complete the test please connect a PM sensor on connector A");
					else break;
				}
			}
		}
	}

	// Led green
	pinMode(pinBLUE, OUTPUT);
	digitalWrite(pinBLUE, HIGH);
	pinMode(pinRED, OUTPUT);
	digitalWrite(pinRED, HIGH);
	pinMode(pinGREEN, OUTPUT);
	digitalWrite(pinGREEN, LOW);

	SerialUSB.println("Test completed OK!!");

}

void setup() {

	SerialUSB.println("Starting...");

	setupPins();

	ledCycle();

	searchPM();

}

void loop() {

}
