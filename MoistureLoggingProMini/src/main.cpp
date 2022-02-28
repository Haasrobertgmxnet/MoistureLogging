#include <Arduino.h>
#include "SoftwareSerial.h"

// SIM card PIN (leave empty, if not defined)
const char simPIN[] = "";//"1503";

// phone number to send SMS: + (plus sign) and country code, for Portugal +351, followed by phone number
#define SMS_TARGET  "+4917680181926"

const int resetPin= 5;
const int sensorPin= SIG_PIN;
const int controlPin= PWR_PIN;
const int espPin= PWR_PIN_ESP32;
const int rxPin= RX_PIN;
const int txPin= TX_PIN;

#define SerialMon Serial

SoftwareSerial SerialAT(txPin, rxPin);

//#define USE_AT_CMDS
#define USE_TINYGSM


#ifdef USE_AT_CMDS
#undef USE_TINYGSM
#endif

#ifdef USE_TINYGSM
// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800      // Modem is SIM800
#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb

#include "TinyGsmClient.h"

#define DUMP_AT_COMMANDS
//#undef DUMP_AT_COMMANDS
#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, Serial);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(SerialAT);
#endif
#endif

ISR(WDT_vect){
  //DON'T FORGET THIS!  Needed for the watch dog timer.  This is called after a watch dog timer timeout - this is the interrupt function called after waking up
}// watchdog interrupt



String state= "Undefined";

void updateSerial()
{
  delay(500);
  while (SerialMon.available()) 
  {
    SerialAT.write(SerialMon.read());//Forward what Serial received to Software Serial Port
  }
  while(SerialAT.available()) 
  {
    SerialMon.write(SerialAT.read());//Forward what Software Serial received to Serial Port
  }
}

inline String wait_for_msg(String msg, int k= 0){
  unsigned len= msg.length();
  while(SerialAT.available()==0);
  String state;
  do{
    SerialMon.print("... " );
    state= SerialAT.readStringUntil('\r');
  }while (state.substring(0,len)!=msg);
  SerialMon.println("\n\nwait_for_msg: " + state);
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

  SerialMon.begin(9600);
  SerialAT.begin(9600);
  delay(1000);

  while (!SerialMon);
  Serial.println("Initializing...");
  delay(1000);

  // SerialAT.println("AT"); //Once the handshake test is successful, it will back to OK
  // updateSerial();
  // SerialAT.println("AT+CSQ"); //Signal quality test, value range is 0-31 , 31 is the best
  // updateSerial();
  // SerialAT.println("AT+CCID"); //Read SIM information to confirm whether the SIM is plugged
  // updateSerial();
  // SerialAT.println("AT+CREG?"); //Check whether it has registered in the network
  // updateSerial();
  // digitalWrite(resetPin,1);
  // delay(1000);
  // digitalWrite(resetPin,0);
  // delay(1000);
}

void loop()
{
  ++k;
  // power on sim 800
  digitalWrite(controlPin, HIGH);
  // read moisture
  ADCSRA |= (1 << 7); // Enable ADC
  auto reading = analogRead(sensorPin); // Read from sensor pin 2
  ADCSRA &= ~(1 << 7); // Disable ADC
  
  auto smsText= "Bodenfeuchte-Wert: " + String(reading);
  SerialMon.println(smsText);
  
  digitalWrite(resetPin,1);
  delay(1000);
  digitalWrite(resetPin,0);
  delay(1000);

#ifdef USE_TINYGSM

  if (!modem.init()) {
    // if (!modem.restart()) {
    SerialMon.println("Failed to restart modem, delaying 10s and retrying");
  }

  String name = modem.getModemName();
  SerialMon.println("Modem Name: " + name);

  String modemInfo = modem.getModemInfo();
  SerialMon.println("Modem Info: " + modemInfo);

  uint8_t  chargeState = 0;
  int8_t   percent     = 0;
  uint16_t milliVolts = 0;
  modem.getBattStats(chargeState, percent, milliVolts);
  SerialMon.println("Battery charge state: " + String(chargeState));
  SerialMon.println("Battery charge 'percent': " + String(percent));
  SerialMon.println("Battery voltage: " + String(milliVolts / 1000.0F));
  
  int csq = modem.getSignalQuality();
  SerialMon.println("Signal quality: " + String(csq));

  if(k<4){
    SerialMon.println("Loop counter: " + String(k));
    auto res = modem.sendSMS(SMS_TARGET, String(smsText));
    updateSerial();
    SerialMon.println("SMS: " + String(res ? "OK" : "fail"));
  }
  updateSerial();
#endif

#ifdef USE_AT_CMDS
  SerialAT.println("AT"); // Initial handshake
  updateSerial();
  SerialAT.println("AT+CMEE=2"); //Check whether it has registered in the network
  updateSerial();
  SerialAT.println("AT+CSQ"); // Signal quality
  updateSerial();
  SerialAT.println("AT+CREG?"); //Check whether it has registered in the network
  updateSerial();
  SerialAT.println("AT+CFUN?"); //PIN
  updateSerial();
  SerialAT.println("AT+CREG?"); //Check whether it has registered in the network
  updateSerial();
  SerialAT.println("AT+CCID"); //Read SIM information to confirm whether the SIM is plugged
  updateSerial();
  // prepare sms
  
  delay(1000);
  SerialMon.println(k);
  if(k<0){
    SerialMon.println(k);
    SerialAT.print("AT+CMGF=1\r");                   //Set the module to SMS mode
    updateSerial();
    SerialAT.print("AT+CMGS=\"+4917680181926\"\r");  //Your phone number don't forget to include your country code, example +212123456789"
    delay(500);
    SerialAT.print(smsText);
    // SerialAT.print("\r"); 
    delay(500);
    SerialAT.print((char)26);
    SerialAT.println(); // end of message command
    delay(2000);
  }
  updateSerial();
#endif
  delay(1000);
  digitalWrite(controlPin, LOW);
  delay(4000);

  SerialMon.println("Go to sleep!");
  // go to sleep
  for(uint8_t i=0;i<24*10;++i){ // 24 hours per day
    for(uint8_t j=0;j<45;++j){ // 450 times 8 secs per hour
    //BOD DISABLE - this must be called right before the __asm__ sleep instruction
    MCUCR |= (3 << 5); //set both BODS and BODSE at the same time
    MCUCR = (MCUCR & ~(1 << 5)) | (1 << 6); //then set the BODS bit and clear the BODSE bit at the same time
    __asm__  __volatile__("sleep");//in line assembler to go to sleep
    }
  }
}

