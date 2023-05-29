#include <ESP8266WiFi.h>

#define RELAY D1

const char* ssid     = "Star Wok";
const char* password = "freedomtoasterbeagleassaultdroneanchor";

// Variable to store info from motion sensor server
String motion_msg;

// Variable to track when we get the TURN_ON message from server
unsigned long motion_time = 0;

// How long to keep light on in minutes
unsigned long num_minutes = 1;

// IP Address of the motion detector WiFi module
IPAddress server(192, 168, 86, 35);
WiFiClient client;


void setup() {

  pinMode(RELAY, OUTPUT);

  Serial.begin(9600);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  client.connect(server, 80);
  motion_msg = client.readStringUntil('\n');
  relay_control(motion_msg);
}

void lamp_delay(unsigned long motion_time, int num_minutes) {
  if (millis() <= motion_time + num_minutes*60000) {
    digitalWrite(RELAY, HIGH);
  }
  else {
    digitalWrite(RELAY, LOW);
  }
} 

void relay_control(String msg) {

  // Record when motion sensor is triggered
  if (msg == "TURN_ON") {
    motion_time = millis();
  }

  // Keep light on for n = num_minutes after last time motion was sensed
  lamp_delay(motion_time, num_minutes);
}
