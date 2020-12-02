#include <Arduino.h>

// RGB Led pins
const uint8_t pinRED 	= 26;		// PA27
const uint8_t pinGREEN 	= 25;		// PB3
const uint8_t pinBLUE 	= 13;		// PA17

// PMS CONNECTOR A
const uint8_t ENABLE_PMS_A = 2; 	// PA14
const uint8_t RX_A = 3;			// PA9 - SERCOM2_ALT/PAD[1]
const uint8_t TX_A = 4;			// PA8 - SERCOM2_ALT/PAD[0]
const uint8_t RESET_PMS_A = 15;		// PB8
const uint8_t PWM_PMS_A = 8; 		// PA6
const uint8_t POWER_PMS_A = 31; 	// PB23
extern Uart SerialPMA_A;

// PMS CONNECTOR B (mapped on Arduino Zero Core Serial1)
const uint8_t ENABLE_PMS_B = 5; 	// PA15
const uint8_t RX_B = 0;				// PA11 - SERCOM0/PAD[3]
const uint8_t TX_B = 1;				// PA10 - SERCOM0/PAD[2]
const uint8_t RESET_PMS_B = 16;		// PB9
const uint8_t PWM_PMS_B = 9; 		// PA7
const uint8_t POWER_PMS_B = 30;		// PB22
#define SerialPMA_B Serial1

// Groove ADC 1
const uint8_t ADC0 = 17; 			// PA04 - A3
const uint8_t ADC1 = 18; 			// PA05 - A4

// Groove ADC 2
const uint8_t ADC2 = 19;			// PB02 - A5
const uint8_t ADC3 = 14; 			// PA02 - A0

// Groove GPIO
const uint8_t GPIO0 = 22; 			// PA12 - SERCOM4_ALT/PAD[0]
const uint8_t GPIO1 = 38; 			// PA13 - SERCOM4_ALT/PAD[1]

// Groove UART
const uint8_t RX0 = 34; 				// PA19 - SERCOM1/PAD[3]
const uint8_t TX0 = 36; 				// PA18 - SERCOM1/PAD[2]
extern Uart SerialGrove;

void setupPins();
