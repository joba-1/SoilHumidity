#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Ticker.h>

#define PROGRAM "Moisture"
#define VERSION "1.5"

#define A0_WET 875
#define A0_DRY 445

#define PERCENT_DRY_LED_BLINK  30
#define PERCENT_DRY_LED_ON  35
#define PERCENT_WET_LED_OFF 40

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

IPAddress apAddr(192, 168, 4, 1); // kind of standard ip for esp APs
IPAddress apMask(255, 255, 255, 0);
DNSServer dnsServer;
ESP8266WebServer webServer(80);

Ticker ticker;

const char msgTemplate[] = "<!DOCTYPE html><html><head><title>%s</title>"
                           "<meta http-equiv=\"refresh\" content=\"10\"></head>"
                           "<body><h1>Humidity: %d%%</h1></body></html>";
char id[sizeof(PROGRAM " " VERSION " hhhhhhhh")];
char msg[sizeof(msgTemplate) + sizeof(id)];

// Return the current humidity value in percent
int getHumidityPercent() {
  return map(constrain( analogRead(A0), A0_DRY, A0_WET ), A0_WET, A0_DRY, 0, 100);
}

// Print humidity to serial and set led on if too dry or off if wet enough
void checkHumidity( void ) {
  int humidity = getHumidityPercent();

  Serial.printf("Humidity: %d%%\n", humidity);

  if( humidity < PERCENT_DRY_LED_BLINK ) {
    ledState = (ledState == Led::ON) ? Led::OFF : Led::ON;   // Urgent to give plants some water
  }
  else if( humidity < PERCENT_DRY_LED_ON ) {
    ledState = Led::ON;   // Hint to give plants some water
  }
  else if( humidity > PERCENT_WET_LED_OFF ) {
    ledState = Led::OFF;  // Soil is moist
  }

  led.set(ledState); // Set always, because Serial or webserver might mess with it
}

// Standard response for any http request: send current humidity
void respond() {
  led.set(ledState == Led::ON ? Led::OFF : Led::ON);
  int humidity = getHumidityPercent();
  snprintf(msg, sizeof(msg), msgTemplate, id, humidity);
  webServer.send(200, "text/html", msg);
  led.set(ledState);
}

// Start wifi AP with captive dns portal and webserver, serial out ticker and alarm led
void setup(void) {
  Serial.begin(115200);

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apAddr, apAddr, apMask);
  snprintf(id, sizeof(id), "%s %s %06x", PROGRAM, VERSION, ESP.getChipId());
  WiFi.softAP(id);

  dnsServer.start(53, "*", apAddr);

  webServer.onNotFound(respond);
  webServer.begin();

  MDNS.begin(PROGRAM);
  MDNS.addService("http", "tcp", 80);

  ticker.attach(1, checkHumidity);

  Serial.printf("\nBooted %s\n", id);
  led.begin(ledState);
}

// Keep servers running
void loop( void ) {
  dnsServer.processNextRequest();
  webServer.handleClient();
}
