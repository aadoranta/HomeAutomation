#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define RELAY D1

// Replace with your network credentials
const char* ssid = "Star Wok";
const char* password = "freedomtoasterbeagleassaultdroneanchor";

// Variable to track when motion is detected
unsigned long motion_time = 0;

// Minutes to keep light on
int num_minutes = 1;

ESP8266WebServer server(80);

void handleRoot() {
  String message = "Hello, ESP8266!";
  server.send(200, "text/plain", message);
}

void relayControl(String motionMsg) {
  if (motionMsg == "ON") {
    digitalWrite(RELAY, HIGH);
  } else {
    digitalWrite(RELAY, LOW);
  }
}

void motionDetection() {
  if (server.method() == HTTP_POST) {
    String motionMsg = server.arg("message");
    relayControl(motionMsg);
    server.send(200, "text/plain", "Motion Message Received");
  }
}

void setup() {

  pinMode(RELAY, OUTPUT);

  Serial.begin(9600);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Print ESP8266 module IP address
  Serial.print("ESP8266 IP address: ");
  Serial.println(WiFi.localIP());

  // Handle root URL
  server.on("/", handleRoot);

  // Handle motion detection requests
  server.on("/motion", motionDetection);

  // Start the server
  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
}
