#include <Arduino.h>

/********************/

// #include <SoftwareSerial.h>
// SoftwareSerial mySerial(11,10);
// const unsigned int wt_ms = 100; // wait ms

// void updateSerial(unsigned int wait_ms){
  
//   String dataString = "";
//   delay(wait_ms);
//   if(mySerial.available()) {
//     Serial.println("updateSerial 1");
//     dataString = mySerial.readString();
//     Serial.println(dataString);
//   }
//   while(Serial.available()) {
//     Serial.println("updateSerial 2");
//     mySerial.write(Serial.read());
//   }
// }

// void setup() {
//   Serial.begin(9600);
//   Serial.println("Send SMS Sketch 1");
//   mySerial.begin(9600);
//   delay(1000);
//   updateSerial(wt_ms);
//   Serial.println("Send SMS Sketch 2");
//   mySerial.println("AT"); // to check if the module is connected
//   updateSerial(wt_ms);
//   // mySerial.println("AT+CPIN=\"xxxx\""); // if Pin is needed
//   // updateSerial(wt_ms);
//   // delay(8000); // maybe needed to setup connection to the network
//   mySerial.println("AT+CCID"); // optional check
//   updateSerial(wt_ms);
//   mySerial.println("AT+CBC"); // optional check
//   updateSerial(wt_ms);
//   mySerial.println("AT+COPS?"); // optional check
//   updateSerial(wt_ms);
//   mySerial.println("AT+CSQ"); // optional check
//   updateSerial(wt_ms);
//   mySerial.println("AT+CREG?"); // optional check
//   updateSerial(wt_ms);
//   mySerial.println("AT+CMGF=1"); // SMS text mode
//   updateSerial(wt_ms);
//   mySerial.println("AT+CMGS=\"491746094125\"");
//   updateSerial(wt_ms);
//   mySerial.print("Hi Wolle, this is a message from your SIM800L Module."); 
//   updateSerial(wt_ms);
//   mySerial.write(26);
//   mySerial.println("");
//   updateSerial(wt_ms);
// }

// void loop() { 
//   updateSerial(0);
// }
  



/********************/
const int resetPin= 5;
const int sensorPin= SIG_PIN;
const int controlPin= PWR_PIN;
const int controlPin2= PWR_PIN2;
const int rxPin= RX_PIN;
const int txPin= TX_PIN;

#define USE_TINYGSM
// #undef USE_TINYGSM

#define USE_AT_CMDS
#undef USE_AT_CMDS

#ifdef USE_TINYGSM
#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb
#endif

#define SerialMon Serial

#include <SoftwareSerial.h>
SoftwareSerial SerialAT(rxPin, txPin);

#ifdef USE_TINYGSM
// See all AT commands, if wanted
#define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon

// Add a reception delay, if needed.
// This may be needed for a fast processor at a slow baud rate.
#define TINY_GSM_YIELD() { delay(2); }
#endif

// Set phone numbers, if you want to test SMS and Calls
#define SMS_TARGET  "+4917680181926"
// #define SMS_TARGET  "+491746094125"

// SIM card PIN (leave empty, if not defined)
const char simPIN[] = "";//"1503";

#ifdef USE_TINYGSM
#include <TinyGsmClient.h>

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(SerialAT);
#endif

#endif

#define SEND_SMS

//#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb

ISR(WDT_vect){
  //DON'T FORGET THIS!  Needed for the watch dog timer.  This is called after a watch dog timer timeout - this is the interrupt function called after waking up
}// watchdog interrupt

String state= "Undefined";

double DisplayValue(uint16_t rawValue){
  double Slope= -0.2584;
  double Intercept= 174.6784;
  return Slope*rawValue + Intercept;
}

void updateSerial(unsigned int wait_ms= 100){
  String dataString = "";
  delay(wait_ms);
  if(SerialAT.available()) {
    dataString = SerialAT.readString();
    SerialMon.println(dataString);
  }
  while(SerialMon.available()) {
    SerialAT.write(SerialMon.read());
  }
}

void SendSMS(bool sendSMS=true)
{
  if(sendSMS==false){
    return;
  }

  SerialAT.write("AT+CSQ\r");
  updateSerial();
  SerialAT.print("AT+CSQ\r");
  SerialAT.println();
  updateSerial();
  SerialAT.print("AT+CREG?\r");
  SerialAT.println();
  updateSerial();
  SerialMon.println("Sending SMS...");               //Show this message on serial monitor
  SerialAT.print("AT+CMGF=1\r");                   //Set the module to SMS mode
  delay(100);
  SerialAT.print("AT+CMGS=\"+4917680181926\"\r");  //Your phone number don't forget to include your country code, example +212123456789"
  delay(500);
  SerialAT.print("SIM800l is working");       //This is the text to send to the phone number, don't make it too long or you have to modify the SoftwareSerial buffer
  delay(500);
  SerialAT.write(26);
  SerialAT.print((char)26);// (required according to the datasheet)
  delay(500);
  SerialAT.println();
  SerialMon.println("Text Sent.");
  delay(500);

}
void setup()
{
  pinMode(controlPin, OUTPUT);
  pinMode(controlPin2, OUTPUT);

  // Save Power by writing all Digital IO LOW - note that pins just need to be tied one way or another, do not damage devices!
  for (int i = 0; i < 16; i++) {
    if(i == sensorPin) continue;
    if(i == rxPin) continue;
    if(i == txPin) continue;
    pinMode(i, OUTPUT);
  }

  // pinMode(txPin, OUTPUT);
  // pinMode(rxPin, INPUT);

  //SETUP WATCHDOG TIMER
  WDTCSR = (24);//change enable and WDE - also resets
  WDTCSR = (33);//prescalers only - get rid of the WDE and WDCE bit
  WDTCSR |= (1<<6);//enable interrupt mode

  //ENABLE SLEEP - this enables the sleep mode
  SMCR |= (1 << 2); //power down mode
  SMCR |= 1;//enable sleep

  ADCSRA &= ~(1 << 7); // Disable ADC
  digitalWrite(controlPin, LOW);
  digitalWrite(controlPin2, HIGH);

  SerialMon.begin(9600);
  delay(10);
  
  // !!!!!!!!!!!
  // Set your reset, enable, power pins here
  // !!!!!!!!!!!
  
  SerialMon.println("Wait...");
  delay(6000);

  SerialAT.begin(9600);
  delay(1000);

  while (!SerialMon);
  Serial.println("Initializing...");
  // delay(1000);

}

void loop()
{
  
  // digitalWrite(txPin, HIGH);
  // digitalWrite(rxPin, HIGH);
  
  // Power on SIM 800 by pull down Gate of p-FET
  digitalWrite(controlPin2, LOW);

  // Power on interface
  digitalWrite(controlPin, HIGH);

  // Read moisture
  uint16_t moist[10];

  for(auto k=0;k<5;++k){
    ADCSRA |= (1 << 7); // Enable ADC
    uint16_t reading = analogRead(sensorPin); // Read from sensor pin 2
    ADCSRA &= ~(1 << 7); // Disable ADC
    moist[k]= reading;
    auto smsText= "Bodenfeuchte-Wert: " + String(reading);
    SerialMon.println(smsText);
    delay(200);
  }
  
  double moistValue= DisplayValue(moist[4]);
  auto smsText= "Bodenfeuchte-Wert in Prozent: " + String(moistValue);
  SerialMon.println(smsText);
  

  digitalWrite(resetPin,1);
  delay(1000);
  digitalWrite(resetPin,0);
  delay(1000);

#ifdef USE_AT_CMDS
  SendSMS(); 
  if (SerialAT.available()){            //Displays on the serial monitor if there's a communication from the module
    SerialMon.write(SerialAT.read()); 
  }
#endif
#ifdef USE_TINYGSM
  if (!modem.init()) {
  // if (!modem.restart()) {
    SerialMon.println("Failed to restart modem, delaying 10s and retrying");
  }

  // Unlock your SIM card with a PIN if needed
  if (strlen(simPIN) && modem.getSimStatus() != 3 ) {
    modem.simUnlock(simPIN);
    SerialMon.println("SIM Unlock");
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

  if (!modem.waitForNetwork(10000L)) {
    SerialMon.println("waitForNetwork fail");
  }
#ifdef SEND_SMS
  if(moistValue < 60.0){
    auto res = modem.sendSMS(SMS_TARGET, String(smsText));
    SerialMon.println("SMS: " + String(res ? "OK" : "fail"));
  }
  
#else
  SerialMon.println("No SEND_SMS");
  delay(500);
#endif
#endif

  delay(1000);
  digitalWrite(controlPin, LOW);
  delay(4000);

  SerialMon.println("Go to sleep!");
  delay(50);
  digitalWrite(txPin, LOW);
  digitalWrite(rxPin, LOW);
  digitalWrite(controlPin2, HIGH);

  // const uint8_t max1 = 88;
  // const uint8_t max2 = 120;
  const uint8_t max1 = 88;
  const uint8_t max2 = 60;

  // go to sleep
  for(uint8_t i=0;i<max1;++i){ // 24 hours per day
    for(uint8_t j=0;j<max2;++j){ // 450 times 8 secs per hour
    //BOD DISABLE - this must be called right before the __asm__ sleep instruction
    MCUCR |= (3 << 5); //set both BODS and BODSE at the same time
    MCUCR = (MCUCR & ~(1 << 5)) | (1 << 6); //then set the BODS bit and clear the BODSE bit at the same time
    __asm__  __volatile__("sleep");//in line assembler to go to sleep
    }
  }
}

