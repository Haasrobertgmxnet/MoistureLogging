# MoistureLogging

## Requirements
* Measure the soil moisture
* Inform the user who is not at the measurement site if the soil moisturec is below a critical value
* Operate the measurements at a rather remote place:
   * There is no electrical current from network
   * There is no WLAN network
## Approach
* Take a MCU board (Arduino, ESP)
* Operate this MCU board vi a LiPo akku
* Send the soil moisture value via SMS via the GSM network to the user
## Final solution
* MCU: Arduino Pro Mini 3.3V which can be operated with a 18650 LiPo Akku 3.7V
* Measurement: Capacitive Soil Moisture Sensor v1.2
* SMS: SIM 800 L Module
* Power management: TP 4056 breakout board
* Capacitors: Low-ESR cpacitors 1000 µF, 10V
* Power switch: NDP6020P p-channel Mosfet
## The Arduino Pro Mini 3.3V
* Most Arduinos need 5V power supply
* This Arduino Pro Mini 3.3V needs 3.3V, more precise it can operated between 2.8V and 5.5V
* Though the layout is simple and reduced it has the powerful Atmega 328P on board, which is also on the Arduino Nano (5V)
* The following modifications in hardware need to be done: https://andreasrohner.at/posts/Electronics/How-to-modify-an-Arduino-Pro-Mini-clone-for-low-power-consumption/
   * remove the power LED to save current
   * remove the voltage regulator
* Software measures to save current: https://www.youtube.com/watch?v=urLSDi7SD8M https://www.kdcircuits.com , Kevin Darrah
   * Disable ADC whenever possible
   * Disable Timer, use Watchdog timer
   * Configure all pins as OUTPUT when possible
## The SIM 800 L Module
* First: This module is _not_ supported for operation in Germany
* Nevertheless it works there
* Very important 1: The module draws up to 2A when sending a SMS over GSM. That means:
   1. The current must be stabilized with a capacitor. I used two Low-ESR capacitors with 1000 µF. Otherwise voltage may fall below 3.4V -the operating voltage- and the SIM 800 L restarts.
   1. Even the thin jumper wires on a breadboard may not be sufficient to provide these current of 2A. I found out that the current supply works better when using own silver-copper wires of 1.2 mm thickness. NB: Wires of this thickness can only be processed/bended with tongs.
* Very important 2: The RX pin must not operated with 3.3V, just 2.1V to 2.8V (or 3.1V max). So the resistor scheme from the SIM 800 L datasheet has to be considered and possibly to be adjusted. Otherwise the module may be damaged.

  
