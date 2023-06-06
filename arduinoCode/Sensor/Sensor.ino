#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

// Motion sensing pin
#define MOTION D1

// Replace with your network credentials
const char* ssid = "Star Wok";
const char* password = "freedomtoasterbeagleassaultdroneanchor";


ESP8266WebServer server(80);

void handleRoot() {
  server.send(200, "text/plain", "Sensor Hello");
}

void getMotion() {
  if (server.method() == HTTP_GET) {
    int motion = digitalRead(MOTION);
    server.send(200, "text/plain", String(motion));
  }
}

void setup() {
  Serial.begin(9600);

  // Initialize Motion sensor
  pinMode(MOTION, INPUT);

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

  // Handle incoming request
  server.on("/", handleRoot);

  // Add a new handler for sending requests
  server.on("/get-motion", getMotion);

  // Start the server
  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
}
