#include <pins.h>
#include "wiring_private.h"

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

void setupPins() {

	SerialUSB.print("Setting up pins... ");

	// RGB led
	pinMode(pinRED, OUTPUT);
	pinMode(pinGREEN, OUTPUT);
	pinMode(pinBLUE, OUTPUT);

	digitalWrite(pinBLUE, HIGH);
	digitalWrite(pinGREEN, HIGH);		// Turn GREEN on
	digitalWrite(pinRED, HIGH);


	// PM_A connector
	pinMode(SET_PMS_A, OUTPUT);
	digitalWrite(SET_PMS_A, HIGH); 		// HIGH: Enable PMS

	pinMode(RST_PMS_A, OUTPUT);
	digitalWrite(RST_PMS_A, HIGH);		// pull this pin down to reset module

	pinMode(PWM_PMS_A, INPUT);

	pinMode(SW_PMSXA, OUTPUT);
	digitalWrite(SW_PMSXA, HIGH);		// HIGH: Turn on power to PMS

	SerialPMA_A.begin(9600);
	pinPeripheral(RX_A, PIO_SERCOM_ALT);
	pinPeripheral(TX_A, PIO_SERCOM_ALT);


	// PM_B connector
	pinMode(SET_PMS_B, OUTPUT);
	digitalWrite(SET_PMS_B, HIGH); 		// HIGH: Enable PMS

	pinMode(RST_PMS_B, OUTPUT);
	digitalWrite(RST_PMS_B, HIGH);		// pull this pin down to reset module

	pinMode(PWM_PMS_B, INPUT);

	pinMode(SW_PMSXB, OUTPUT);
	digitalWrite(SW_PMSXB, HIGH);		// HIGH: Turn on power to PMS

	SerialPMA_B.begin(9600);


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

	SerialUSB.println("OK");

}
