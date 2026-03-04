/*
Sample Code for DS18B20 Sensor
This example code reads temperature from a single DS18B20 sensor. Connect the sensor's data pin to a GPIO pin on the ESP32 (e.g., GPIO 4) with a 4.7k Ohm pull-up resistor to VCC. 

Key Functions
OneWire oneWire(PIN);: Instantiates the OneWire object on the specified GPIO pin.
sensors.begin();: Initializes the Dallas Temperature library and searches for connected devices.
sensors.requestTemperatures();: Sends a command to all devices to perform a temperature conversion.
sensors.getTempCByIndex(0);: Reads the temperature from the first device found on the bus. 
www.industrialshields.com
www.industrialshields.com
 +4

https://docs.arduino.cc/libraries/onewire

https://github.com/milesburton/Arduino-Temperature-Control-Library


 */


#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into GPIO 4 (change to your specific pin)
#define ONE_WIRE_BUS 4

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

void setup(void) {
  Serial.begin(115200);
  Serial.println("Dallas Temperature IC Control Library Demo");
  // Start up the library
  sensors.begin();
}

void loop(void) {
  // Call sensors.requestTemperatures() to issue a global temperature request to all devices on the bus
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  
  // After we got the temperatures, we print them in Celsius and Fahrenheit
  Serial.print("Temperature Celsius: ");
  Serial.print(sensors.getTempCByIndex(0)); // Get the temperature in Celsius
  Serial.print(" Temperature Fahrenheit: ");
  Serial.println(sensors.getTempFByIndex(0)); // Get the temperature in Fahrenheit
  
  delay(2000);
}
