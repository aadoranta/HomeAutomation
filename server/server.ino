#include <ESP8266WiFi.h>

// Read motion sensor at Pin 0 (D1), output light from pin 2 (D2)
// 0, 2 PINS for small esp8266; D1, D2 for larger esp8266
// If uploading to breadboard compatible esp8266; must use NodeMCU board

// Motion signal to send to client
#define MOTION D1
// Button to turn off motion sensing control
#define SWITCH D6
// LED to identify when motion sensing is off
#define LED D7

// Replace with your network credentials
const char* ssid     = "Star Wok";
const char* password = "freedomtoasterbeagleassaultdroneanchor";

bool buttonFlag = LOW;
bool previousButtonFlag = LOW;
bool switchState = LOW;

// Set web server port number to 80
WiFiServer server(80);

IPAddress ip(192, 168, 86, 35);
IPAddress gateway(192, 168, 86, 1);
IPAddress subnet(255, 255, 255, 0);

// Flag to check if motion is detected
int motion = 0;

void setup() {

  Serial.begin(9600);

  pinMode(MOTION, INPUT);
  pinMode(SWITCH, INPUT);
  pinMode(LED, OUTPUT);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {

  WiFiClient client = server.available();
  if (client) {
    while (client.connected()) {

      if (buttonState()){
        Serial.println("HEllo");
        motion = digitalRead(MOTION);
        if (motion == 1) {
          Serial.println("Motion");
          client.println("TURN_ON\n");
          }
        } else {
          client.println("TURN_ON\n");
        }

      client.flush();
    }
  }
}

bool buttonState() {
  // Get current button state
  buttonFlag = digitalRead(SWITCH);

  // Check if the button state has changed
  if (buttonFlag != previousButtonFlag) {
    // Check if the button is pressed (LOW state)
    if (buttonFlag == HIGH) {
      // Toggle the LED state
      switchState  = !switchState;
      digitalWrite(LED, switchState);
    }
  }

  // Update the previous button state
  previousButtonFlag = buttonFlag;

  return switchState;
}
