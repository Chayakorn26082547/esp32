#include <WiFi.h>
#include <MicroGear.h>
#include <HTTPClient.h>

// -------- NETPIE credentials --------
#define APPID   "1d4c1ec7-9efe-4b3e-a9fd-903471a73f39"
#define KEY     "FYJpS15HcvRvF7KtCwrnqC2t7bLKsj2R"
#define SECRET  "z7jBNvj4JckL8Vxsu885jGC7EyKwi5xa"
#define ALIAS   "ESP32_NodeMCU"

// -------- Wi-Fi credentials --------
const char* ssid = "YourWiFiName";         // Replace with your WiFi name
const char* password = "YourWiFiPassword"; // Replace with your WiFi password

// -------- MicroGear setup --------
WiFiClient client;
MicroGear microgear(client);

// -------- Timer for periodic HTTP call --------
unsigned long lastTime = 0;
const unsigned long interval = 5 * 60 * 1000; // 5 minutes in milliseconds

// -------- Function to send JSON to HTTP API --------
void sendJsonToAPI(String message) {
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin("http://http.aaa/api"); // Replace with your API endpoint
    http.addHeader("Content-Type", "application/json");

    String jsonBody = "{\"device\":\"ESP32_NodeMCU\",\"message\":\"" + message + "\"}";

    int httpResponseCode = http.POST(jsonBody);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      Serial.println("Response: " + response);
    } else {
      Serial.print("Error sending POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}

// -------- NETPIE Event Handlers --------
void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.println(">>> Connected to NETPIE");
  microgear.setAlias(ALIAS);
  sendJsonToAPI("ESP32 connected to NETPIE"); // Notify on connect
}

void onMessage(char *topic, uint8_t* msg, unsigned int msglen) {
  Serial.print("Incoming message on topic ");
  Serial.print(topic);
  Serial.print(": ");
  String message = "";
  for (int i = 0; i < msglen; i++) {
    message += (char)msg[i];
  }
  Serial.println(message);

  // Optionally send message to HTTP API
  sendJsonToAPI("Message received: " + message);
}

void onPresent(char *alias) {
  Serial.print("Device online: ");
  Serial.println(alias);
  sendJsonToAPI("Device online: " + String(alias));
}

void onAbsent(char *alias) {
  Serial.print("Device offline: ");
  Serial.println(alias);
  sendJsonToAPI("Device offline: " + String(alias));
}

void onError(char *error) {
  Serial.print("NETPIE Error: ");
  Serial.println(error);
  sendJsonToAPI("NETPIE Error: " + String(error));
}

// -------- WiFi setup --------
void setupWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// -------- Arduino setup --------
void setup() {
  Serial.begin(115200);
  delay(1000);

  setupWiFi();

  // Bind NETPIE event hooks
  microgear.on(CONNECTED, onConnected);
  microgear.on(MESSAGE, onMessage);
  microgear.on(PRESENT, onPresent);
  microgear.on(ABSENT, onAbsent);
  microgear.on(ERROR, onError);

  // Initialize MicroGear and connect to NETPIE
  microgear.init(KEY, SECRET, ALIAS);
  Serial.println("Connecting to NETPIE...");
  microgear.connect(APPID);
}

// -------- Main loop --------
void loop() {
  // Handle NETPIE messages
  if (microgear.connected()) {
    microgear.loop();
  } else {
    Serial.println("Reconnecting to NETPIE...");
    microgear.connect(APPID);
  }

  // Periodic HTTP API call every 5 minutes
  if (millis() - lastTime > interval) {
    lastTime = millis();
    sendJsonToAPI("Periodic update every 5 minutes");
  }
}