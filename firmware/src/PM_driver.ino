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

		case PM_START:
		{
				SerialPMA_A.begin(9600);
				pinPeripheral(RX_A, PIO_SERCOM_ALT);	// PMA_A serial port
				pinPeripheral(TX_A, PIO_SERCOM_ALT);	// PMA_A serial port
				pmA.begin();
				delay(1000);

				SerialPMA_B.begin(9600);
				pmB.begin();
				delay(3000);

				wichCommand = command;
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
	case PM_START:
	{
#ifdef debug_PM
			SerialUSB.println("Starting PM sensor...");
#endif
			if (SerialPMA_A.available() || SerialPMA_B.available()) Wire.write(1);
			else {
#ifdef debug_PM
			SerialUSB.println("ERROR Starting PM sensor!!!");
#endif
				Wire.write(0);
				pmA.stop();
				pmB.stop();
			}
			break;
	}
	case GET_PMA:
	{
#ifdef debug_PM
			SerialUSB.println("Sending PMA values...");
#endif
			if (pmA.active) for (uint8_t i=0; i<valuesSize; i++) Wire.write(pmA.values[i]);
			else for (uint8_t i=0; i<valuesSize; i++) Wire.write(255);
			break;
	}
	case GET_PMB:
	{
#ifdef debug_PM
			SerialUSB.println("Sending PMB values...");
#endif
			if (pmB.active) for (uint8_t i=0; i<valuesSize; i++) Wire.write(pmB.values[i]);
			else for (uint8_t i=0; i<valuesSize; i++) Wire.write(255);
			break;
	}
	case GET_PM_AVG:
	{
#ifdef debug_PM
			SerialUSB.println("Sending average values...");
#endif
			uint8_t toSendValues[valuesSize];

			// Average both readings
	if (pmA.active && pmB.active) {

				uint16_t bothPm1 = (pmA.pm1 + pmB.pm1) / 2;
				uint16_t bothPm25 = (pmA.pm25 + pmB.pm25) / 2;
				uint16_t bothPm10 = (pmA.pm10 + pmB.pm10) / 2;
				uint16_t bothPn03 = (pmA.pn03 + pmB.pn03) / 2;
				uint16_t bothPn05 = (pmA.pn05 + pmB.pn05) / 2;
				uint16_t bothPn1 = (pmA.pn1 + pmB.pn1) / 2;
				uint16_t bothPn25 = (pmA.pn25 + pmB.pn25) / 2;
				uint16_t bothPn5 = (pmA.pn5 + pmB.pn5) / 2;
				uint16_t bothPn10 = (pmA.pn10 + pmB.pn10) / 2;

				toSendValues[0] = bothPm1 >> 8;
				toSendValues[1] = bothPm1 & 0x00FF;
				toSendValues[2] = bothPm25 >> 8;
				toSendValues[3] = bothPm25 & 0x00FF;
				toSendValues[4] = bothPm10 >> 8;
				toSendValues[5] = bothPm10 & 0x00FF;
				toSendValues[6] = bothPn03 >> 8;
				toSendValues[7] = bothPn03 & 0x00FF;
				toSendValues[8] = bothPn05 >> 8;
				toSendValues[9] = bothPn05 & 0x00FF;
				toSendValues[10] = bothPn1 >> 8;
				toSendValues[11] = bothPn1 & 0x00FF;
				toSendValues[12] = bothPn25 >> 8;
				toSendValues[13] = bothPn25 & 0x00FF;
				toSendValues[14] = bothPn5 >> 8;
				toSendValues[15] = bothPn5 & 0x00FF;
				toSendValues[16] = bothPn10 >> 8;
				toSendValues[17] = bothPn10 & 0x00FF;

				for (uint8_t i=0; i<valuesSize; i++) {
					Wire.write(toSendValues[i]);
				}

			// Otherwise send only the active one	
			} else if (pmA.active) for (uint8_t i=0; i<valuesSize; i++) Wire.write(pmA.values[i]);
			else if (pmB.active) for (uint8_t i=0; i<valuesSize; i++) Wire.write(pmB.values[i]);

			// Or send 255 as ERROR code
			else for (uint8_t i=0; i<valuesSize; i++) Wire.write(255);

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
			if (!groveGps.getReading()) Wire.write(0);
			else {
				for (uint8_t i=0; i<groveGps.DATA_LEN; i++) Wire.write(groveGps.data[i]);
			}
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

		// temp
		groveGps.getReading();
	}

	if (groveGps.enabled) {
		while (SerialGrove.available()) {
			/* char c = SerialGrove.read(); */
			groveGps.encode(SerialGrove.read());
			/* groveGps.encode(c); */
			/* SerialUSB.print(c); */
		}
	}
}
