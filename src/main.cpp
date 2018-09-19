#include <Arduino.h>

// Just to make sure it is off
#include <ESP8266WiFi.h>

#define LED_PIN D4

// NodeMCU led D4 is inverted
#define LED_ON  LOW
#define LED_OFF HIGH

bool ledState = LED_OFF;

void setup(void) {
  Serial.begin(115200);
  WiFi.mode(WIFI_OFF);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("\nBooted.");
  digitalWrite(LED_PIN, ledState); // Serial messes with led state
}

void loop( void ) {
  // Capacitive sensor pin on A0 yields 875 in water and 445 in air
  // Translate this to a range from 100% (wet) to 0% (dry)
  int humidity = map(constrain( analogRead(A0), 445, 875 ), 875, 445, 0, 100);

  Serial.print("humidity: ");
  Serial.println(humidity);

  if( humidity < 20 ) {
    ledState = LED_ON;   // Time to give plants some water
  }
  else if( humidity > 25 ) {
    ledState = LED_OFF;  // Soil is moist
  }
  digitalWrite(LED_PIN, ledState);

  delay(1*1000);
}
