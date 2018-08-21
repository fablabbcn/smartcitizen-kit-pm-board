#include "wiring_private.h"
#include <Arduino.h>
#include <Wire.h>

#include <PM_driver.h>

#define I2C_ADDRESS 0x02

// Hardware Serial UART PM-A
Uart SerialPMA_A (&sercom2, RX_A, TX_A, SERCOM_RX_PAD_1, UART_TX_PAD_0);
void SERCOM2_Handler() {
	SerialPMA_A.IrqHandler();
}

// Hardware Serial UART PM-B (already setup by arduino core as Serial1 and renamed in headers file)

// Hardware Serial UART GroveUART
Uart SerialGrove (&sercom1, RX0, TX0, SERCOM_RX_PAD_3, UART_TX_PAD_2);
void SERCOM1_Handler() {
	SerialGrove.IrqHandler();
}

PMsensor pmA(&SerialPMA_A, POWER_PMS_A, ENABLE_PMS_A, RESET_PMS_A);
PMsensor pmB(&SerialPMA_B, POWER_PMS_B, ENABLE_PMS_B, RESET_PMS_B);

uint8_t wichCommand = GET_PMA;

void setup()
{

	// RGB led
	pinMode(pinRED, OUTPUT);
	pinMode(pinGREEN, OUTPUT);
	pinMode(pinBLUE, OUTPUT);

	digitalWrite(pinBLUE, LOW);
	digitalWrite(pinGREEN, HIGH);
	digitalWrite(pinRED, HIGH);

	// PM_A connector
	pinMode(ENABLE_PMS_A, OUTPUT);
	pinMode(POWER_PMS_A, OUTPUT);
	pinMode(RESET_PMS_A, OUTPUT);

	digitalWrite(POWER_PMS_A, LOW);
	digitalWrite(RESET_PMS_A, HIGH);

	pinMode(PWM_PMS_A, INPUT);

	// PM_B connector
	pinMode(ENABLE_PMS_B, OUTPUT);
	pinMode(POWER_PMS_B, OUTPUT);
	pinMode(RESET_PMS_B, OUTPUT);

	digitalWrite(POWER_PMS_B, LOW);
	digitalWrite(RESET_PMS_B, HIGH);

	pinMode(PWM_PMS_B, INPUT);

	// ADC'S
	pinMode(ADC0, INPUT);
	pinMode(ADC1, INPUT);
	pinMode(ADC2, INPUT);
	pinMode(ADC3, INPUT);

	// GPIO as output
	pinMode(GPIO0, OUTPUT);
	pinMode(GPIO1, OUTPUT);

	// Groove UART
	SerialGrove.begin(115200);
	pinPeripheral(RX0, PIO_SERCOM);
	pinPeripheral(TX0, PIO_SERCOM);

	Wire.begin(I2C_ADDRESS);
	Wire.onReceive(receiveEvent);
	Wire.onRequest(requestEvent);

}
void receiveEvent(int howMany)
{

	byte command = 99;
	if (Wire.available()) command = Wire.read();

	switch(command) {

		case PM_START:
		{
				SerialPMA_A.begin(9600);
				pinPeripheral(RX_A, PIO_SERCOM_ALT);	// PMA_A serial port
				pinPeripheral(TX_A, PIO_SERCOM_ALT);	// PMA_A serial port
				pmA.begin();

				delay(2000);

				SerialPMA_B.begin(9600);
				pmB.begin();
				break;
		}
		case PM_STOP:
		{
				pmA.stop();
				pmB.stop();
				break;
		}
		case GET_PM_AVG:
		case GET_PMA:
		case GET_PMB:
		{
				wichCommand = command;
				break;
		}
	}
	/* bool t = digitalRead(pinBLUE); */
	/* digitalWrite(pinBLUE, !t); */
}
void requestEvent()
{
	switch (wichCommand)
{

	case GET_PMA:
	{
			for (uint8_t i=0; i<6; i++) {
				Wire.write(pmA.values[i]);
			}
			break;
	}
	case GET_PMB:
	{
			for (uint8_t i=0; i<6; i++) {
				Wire.write(pmB.values[i]);
			}
			break;
	}
	case GET_PM_AVG:
	{
			bool pmAactive = false;
			bool pmBactive = false;
			uint8_t toSendValues[6] = {0,0,0,0,0,0};	// 6 bytes 0:1->pm1, 2:3->pm25, 4:5->pm10


			// Check if both are active
			if (millis() - pmA.lastReading < 10000) pmAactive = true;
			if (millis() - pmB.lastReading < 10000) pmBactive = true;


			// Send values 	(6 bytes 0:1->pm1, 2:3->pm25, 4:5->pm10)
			// Average both readings
			if (pmAactive && pmBactive) {

				uint16_t bothPm1 = (pmA.pm1 + pmB.pm1) / 2;
				uint16_t bothPm25 = (pmA.pm25 + pmB.pm25) / 2;
				uint16_t bothPm10 = (pmA.pm10 + pmB.pm10) / 2;

				toSendValues[0] = bothPm1 >> 8;
				toSendValues[1] = bothPm1 & 0x00FF;
				toSendValues[2] = bothPm25 >> 8;
				toSendValues[3] = bothPm25 & 0x00FF;
				toSendValues[4] = bothPm10 >> 8;
				toSendValues[5] = bothPm10 & 0x00FF;

				for (uint8_t i=0; i<6; i++) {
					Wire.write(toSendValues[i]);
				}

				// Otherwise send only the active one	
			} else if (pmAactive) {
				for (uint8_t i=0; i<6; i++) {
					Wire.write(pmA.values[i]);
				}
			} else if (pmBactive) {
				for (uint8_t i=0; i<6; i++) {
					Wire.write(pmB.values[i]);
				}
			} else {
				// No PM active, send 0 for now TODO error code to report that no device is available
				for (uint8_t i=0; i<6; i++) {
					Wire.write(0);
				}
			}

			break;
	}
}
}

void loop()
{
	if (millis() % 1000 == 0) {
		pmA.update();
		pmB.update();
	}
}
