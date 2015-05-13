// Simple binary switch example 
// Connect button or door/window reed switch between 
// digitial I/O pin 3 (BUTTON_PIN below) and GND.

#include "MySensor.h"
#include <SPI.h>
//#include <Bounce2.h>

#define CHILD_ID 1
#define BUTTON_PIN  3  // Arduino Digital I/O pin for button/reed switch

MySensor gw;
//Bounce debouncer = Bounce(); 
int oldValue=-1;

// Change to V_LIGHT if you use S_LIGHT in presentation below
MyMessage msg(CHILD_ID,V_TRIPPED);
MyMessage msgtemp(2,V_TEMP);
MyMessage msgsupply(3,V_VOLTAGE);
MyMessage msgcnt(4,V_VAR1);
const int buttonPin = PUSH2;     // the number of the pushbutton pin
const int ledPin =  RED_LED;      // the number of the LED pin
uint8_t cnt=0;
void setup()  
{  
  
  gw.begin(NULL,23);

 // Setup the button
  pinMode(ledPin, OUTPUT);      
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT_PULLUP);     
  
  gw.present(CHILD_ID, S_DOOR);  
  gw.present(2, S_TEMP);  
  gw.present(3, S_ARDUINO_NODE);  
  gw.present(4, S_ARDUINO_NODE);  
    analogReference(INTERNAL1V5); 
   analogRead(TEMPSENSOR); // first reading usually wrong 

}


//  Check if digital input has changed and send in new value
void loop() 
{
  int value = digitalRead(buttonPin);
  digitalWrite(ledPin, !value);  

  if (value != oldValue) {
     // Send in the new value
     gw.send(msg.set(value==HIGH ? 1 : 0));
     oldValue = value;
  }
  gw.sleep(3000);
  uint32_t x=(((uint32_t)analogRead(TEMPSENSOR)*27069 - 18169625) *10 >> 16);
  gw.send(msgtemp.set(x));
  uint16_t y = Msp430_GetSupplyVoltage();
  gw.send(msgsupply.set(y));
  gw.sendBatteryLevel(y/50);
  digitalWrite(ledPin,  gw.send(msgcnt.set(cnt++)));
  sleep(250);
  //gw.sleep(250); not working
  digitalWrite(ledPin, LOW);  
} 

uint16_t Msp430_GetSupplyVoltage(void)
{
	uint16_t raw_value;
	// first attempt - measure Vcc/2 with 1.5V reference (Vcc < 3V )
	ADC10CTL0 = SREF_1 | REFON | ADC10SHT_2 | ADC10SR | ADC10ON;
	ADC10CTL1 = INCH_11 | SHS_0 | ADC10DIV_0 | ADC10SSEL_0;
	// start conversion and wait for it
	ADC10CTL0 |= ENC | ADC10SC;
	while (ADC10CTL1 & ADC10BUSY) ;
	// stop conversion and turn off ADC
	ADC10CTL0 &= ~ENC;
	ADC10CTL0 &= ~(ADC10IFG | ADC10ON | REFON);
	raw_value = ADC10MEM;
	// check for overflow
	if (raw_value == 0x3ff) {
		// switch range - use 2.5V reference (Vcc >= 3V)
		ADC10CTL0 = SREF_1 | REF2_5V | REFON | ADC10SHT_2 | ADC10SR | ADC10ON;
		// start conversion and wait for it
		ADC10CTL0 |= ENC | ADC10SC;
		while (ADC10CTL1 & ADC10BUSY) ;
		raw_value = ADC10MEM;
		// end conversion and turn off ADC
		ADC10CTL0 &= ~ENC;
		ADC10CTL0 &= ~(ADC10IFG | ADC10ON | REFON);
		// convert value to mV
		return ((uint32_t)raw_value * 5000) / 1024;
	} else
		return ((uint32_t)raw_value * 3000) / 1024;
}

