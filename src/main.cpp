#include <Arduino.h>

#include <ESP8266WiFi.h>

#define PROGRAM "Moisture"
#define VERSION "1.0"

#define A0_WET 875
#define A0_DRY 445

#define PERCENT_DRY_LED_ON  25
#define PERCENT_WET_LED_OFF 30

template < bool Inverted > class TLed {
public:
  typedef enum { OFF = false, ON = true } t_state;

  TLed( int pin ) : _pin(pin) {};

  void on( void ) const { digitalWrite(_pin, Inverted ? LOW : HIGH ); };
  void off( void ) const { digitalWrite(_pin, Inverted ? HIGH : LOW ); };
  void set( t_state state ) const { if( state ) on(); else off(); };
  void begin( t_state state = OFF ) const { pinMode(_pin, OUTPUT); set(state); };

private:
  int _pin;
};

typedef class TLed < true > Led;

Led led(D4);
Led::t_state ledState = Led::OFF;

void setup(void) {
  Serial.begin(115200);
  WiFi.mode(WIFI_OFF);
  Serial.printf("\nBooted %s %s\n", PROGRAM, VERSION);
  led.begin(ledState);
}

void loop( void ) {
  // Capacitive sensor pin on A0 yields 875 in water and 445 in air
  // Translate this to a range from 100% (wet) to 0% (dry)
  int humidity = map(constrain( analogRead(A0), A0_DRY, A0_WET ), A0_WET, A0_DRY, 0, 100);

  Serial.print("humidity %: ");
  Serial.println(humidity);

  if( humidity < PERCENT_DRY_LED_ON ) {
    ledState = Led::ON;   // Time to give plants some water
  }
  else if( humidity > PERCENT_WET_LED_OFF ) {
    ledState = Led::OFF;  // Soil is moist
  }

  led.set(ledState);

  delay(1*1000);
}
