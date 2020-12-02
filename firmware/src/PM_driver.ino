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

volatile uint8_t wichCommand = GET_PMA;

Sck_DallasTemp dallasTemp;

GrooveGps groveGps;

uint32_t timer = 0;

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
	SerialGrove.begin(9600);
	pinPeripheral(RX0, PIO_SERCOM);
	pinPeripheral(TX0, PIO_SERCOM);

	// temp
	groveGps.start();

	Wire.begin(I2C_ADDRESS);
	Wire.onReceive(receiveEvent);
	Wire.onRequest(requestEvent);

#ifdef debug_PM
	SerialUSB.begin(115200);
	while (!SerialUSB);
	SerialUSB.println("Starting...");
#endif
}
void receiveEvent(int howMany)
{
	byte command = 99;
	if (Wire.available()) command = Wire.read();

	switch(command) {

		case START_PMA:
		{
				SerialPMA_A.begin(9600);
				pinPeripheral(RX_A, PIO_SERCOM_ALT);	// PMA_A serial port
				pinPeripheral(TX_A, PIO_SERCOM_ALT);	// PMA_A serial port
				pmA.begin();

				// Wait for PM response or timeout
				uint32_t now = millis();
				while (millis() - now < 5000) {
					if (SerialPMA_A.available()) break;
				}

				wichCommand = command;
				break;
		}
		case START_PMB:
		{
				SerialPMA_B.begin(9600);
				pmB.begin();

				// Wait for PM response or timeout
				uint32_t now = millis();
				while (millis() - now < 5000) {
					if (SerialPMA_B.available()) break;
				}
				
				wichCommand = command;
				break;
		}
		case STOP_PMA: { 
			pmA.stop();
			break;
		}
		case STOP_PMB: { 
			pmB.stop();
			break;
		}
		case GET_PMA:
		case GET_PMB:
		case DALLASTEMP_START:
		case DALLASTEMP_STOP:
		case GET_DALLASTEMP:
		case GROVEGPS_START:
		case GROVEGPS_STOP:
		case GROVEGPS_GET:
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
	case START_PMA:
	{
#ifdef debug_PM
			SerialUSB.println("Starting PMA sensor...");
#endif
			if (SerialPMA_A.available()) Wire.write(1);
			else {
#ifdef debug_PM
			SerialUSB.println("ERROR Starting PMA sensor!!!");
#endif
				Wire.write(0);
				pmA.stop();
			}
			break;
	}
	case START_PMB:
	{
#ifdef debug_PM
			SerialUSB.println("Starting PMB sensor...");
#endif
			if (SerialPMA_B.available()) Wire.write(1);
			else {
#ifdef debug_PM
			SerialUSB.println("ERROR Starting PMB sensor!!!");
#endif
				Wire.write(0);
				pmB.stop();
			}
			break;
	}
	case GET_PMA:
	{
#ifdef debug_PM
			SerialUSB.println("PMA values requested....");
#endif
			if (pmA.active) {
#ifdef debug_PM
				SerialUSB.println("Sending PMA values...");
#endif
				for (uint8_t i=0; i<valuesSize; i++) Wire.write(pmA.values[i]);
			} else {
#ifdef debug_PM
				SerialUSB.println("PMA is not active, sending error code...");
#endif
				for (uint8_t i=0; i<valuesSize; i++) Wire.write(255);
			}
			break;
	}
	case GET_PMB:
	{
#ifdef debug_PM
			SerialUSB.println("PMB values requested....");
#endif
			if (pmB.active) {
#ifdef debug_PM
				SerialUSB.println("Sending PMB values...");
#endif
				for (uint8_t i=0; i<valuesSize; i++) Wire.write(pmB.values[i]);
			} else {
#ifdef debug_PM
				SerialUSB.println("PMB is not active, sending error code...");
#endif
				for (uint8_t i=0; i<valuesSize; i++) Wire.write(255);
			}
			break;
	}
	case DALLASTEMP_START:
	{
			uint8_t result = dallasTemp.start();
			Wire.write(result);
			break;
	}
	case DALLASTEMP_STOP:
	{
			uint8_t result = dallasTemp.stop();
			Wire.write(result);
			break;
	}
	case GET_DALLASTEMP:
	{
			if (!dallasTemp.getReading()) dallasTemp.uRead.fval = -9999;
			for (uint8_t i=0; i<4; i++) Wire.write(dallasTemp.uRead.b[i]);
			break;
	}
	case GROVEGPS_START:
	{
			uint8_t result = groveGps.start();
			Wire.write(result);
			break;
	}
	case GROVEGPS_STOP:
	{
			uint8_t result = groveGps.stop();
			Wire.write(result);
			break;
	}
	case GROVEGPS_GET:
	{
			groveGps.getReading();
			for (uint8_t i=0; i<groveGps.DATA_LEN; i++) Wire.write(groveGps.data[i]);
			break;
	}
}
}

void loop()
{
	if (millis() - timer > 1000) {
		pmA.update();
		pmB.update();
		timer = millis();
	}

	if (groveGps.started) {
		while (SerialGrove.available()) groveGps.encode(SerialGrove.read());
	}
}
