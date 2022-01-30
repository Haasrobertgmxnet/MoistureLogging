#include <Arduino.h>
#include "SoftwareSerial.h"

const int sensorPin= SIG_PIN;
const int controlPin= PWR_PIN;
const int espPin= PWR_PIN_ESP32;
const int rxPin= RX_PIN;
const int txPin= TX_PIN;

ISR(WDT_vect){
  //DON'T FORGET THIS!  Needed for the watch dog timer.  This is called after a watch dog timer timeout - this is the interrupt function called after waking up
}// watchdog interrupt

SoftwareSerial mySerial(txPin, rxPin);

String state= "Undefined";

void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    mySerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(mySerial.available()) 
  {
    Serial.write(mySerial.read());//Forward what Software Serial received to Serial Port
  }
}

inline String wait_for_msg(String msg, int k= 0){
  unsigned len= msg.length();
  while(mySerial.available()==0);
  String state;
  do{
    Serial.print("... " );
    state= mySerial.readStringUntil('\r');
  }while (state.substring(0,len)!=msg);
  Serial.println("\n\nwait_for_msg: " + state);
  return state;
}

int k= 0;
void setup()
{
  pinMode(controlPin, OUTPUT);

  // Save Power by writing all Digital IO LOW - note that pins just need to be tied one way or another, do not damage devices!
  for (int i = 0; i < 16; i++) {
    if(i == sensorPin) continue;
    if(i == rxPin) continue;
    if(i == txPin) continue;
    pinMode(i, OUTPUT);
  }

  //SETUP WATCHDOG TIMER
  WDTCSR = (24);//change enable and WDE - also resets
  WDTCSR = (33);//prescalers only - get rid of the WDE and WDCE bit
  WDTCSR |= (1<<6);//enable interrupt mode

  //ENABLE SLEEP - this enables the sleep mode
  SMCR |= (1 << 2); //power down mode
  SMCR |= 1;//enable sleep

  ADCSRA &= ~(1 << 7); // Disable ADC
  digitalWrite(controlPin, LOW);

  Serial.begin(9600);
  mySerial.begin(9600);

  while (!Serial);
  Serial.println("Initializing...");
  delay(1000);

  digitalWrite(controlPin, HIGH);
  mySerial.println("AT"); //Once the handshake test is successful, it will back to OK
  updateSerial();
  mySerial.println("AT+CSQ"); //Signal quality test, value range is 0-31 , 31 is the best
  updateSerial();
  mySerial.println("AT+CCID"); //Read SIM information to confirm whether the SIM is plugged
  updateSerial();
  mySerial.println("AT+CREG?"); //Check whether it has registered in the network
  updateSerial();
  // digitalWrite(controlPin, LOW);
}

void loop()
{
  ++k;
  // power on simm 800
  digitalWrite(controlPin, HIGH);

  // read moisture
  ADCSRA |= (1 << 7); // Enable ADC
  auto reading = analogRead(sensorPin); // Read from sensor pin 2
  ADCSRA &= ~(1 << 7); // Disable ADC
  auto smsText= "Bodenfeuchte-Wert: " + String(reading);

  // prepare sms 
  mySerial.println("AT"); //Once the handshake test is successful, it will back to OK
  updateSerial();
  mySerial.println("AT+CSQ"); //Signal quality test, value range is 0-31 , 31 is the best
  updateSerial();
  mySerial.println("AT+CCID"); //Read SIM information to confirm whether the SIM is plugged
  updateSerial();
  mySerial.println("AT+CREG?"); //Check whether it has registered in the network
  updateSerial();
  // see: https://forums.quectel.com/t/cpin-not-inserted-mc60/6922
  mySerial.println("AT+CFUN=0"); //Signal quality test, value range is 0-31 , 31 is the best
  updateSerial();
  mySerial.println("AT+CFUN=1"); //Read SIM information to confirm whether the SIM is plugged
  updateSerial();
  mySerial.println("AT+CPIN?"); //Check whether it has registered in the network
  updateSerial();
  delay(1000);
  Serial.println(k);
  if(k<3){
    Serial.println(k);
    mySerial.print("AT+CMGF=1\r");                   //Set the module to SMS mode
    updateSerial();
    mySerial.print("AT+CMGS=\"+4917680181926\"\r");  //Your phone number don't forget to include your country code, example +212123456789"
    updateSerial();
    mySerial.print(smsText);
    updateSerial();
  }
  delay(2000);
  digitalWrite(controlPin, LOW);
  delay(4000);

  Serial.println("Go to seep!");
  // go to sleep
  for(int i=0;i<3;++i){
    //BOD DISABLE - this must be called right before the __asm__ sleep instruction
    MCUCR |= (3 << 5); //set both BODS and BODSE at the same time
    MCUCR = (MCUCR & ~(1 << 5)) | (1 << 6); //then set the BODS bit and clear the BODSE bit at the same time
    __asm__  __volatile__("sleep");//in line assembler to go to sleep
  }
}

