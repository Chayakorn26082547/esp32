#include <WiFi.h>
#include <MicroGear.h>
#include <HTTPClient.h>

// -------- NETPIE credentials --------
#define APPID   "1d4c1ec7-9efe-4b3e-a9fd-903471a73f39"
#define KEY     "FYJpS15HcvRvF7KtCwrnqC2t7bLKsj2R"
#define SECRET  "z7jBNvj4JckL8Vxsu885jGC7EyKwi5xa"
#define ALIAS   "ESP32_NodeMCU"

// -------- Wi-Fi credentials --------
const char* ssid = "YourWiFiName";
const char* password = "YourWiFiPassword";

// -------- MicroGear setup --------
WiFiClient client;
MicroGear microgear(client);

// -------- Timer for periodic HTTP call --------
unsigned long lastTime = 0;
const unsigned long interval = 5 * 60 * 1000; // 5 minutes

// -------- Global variable to store last temperature --------
float lastTemperature = 0.0;

// -------- Function to send JSON to HTTP API --------
void sendTemperatureToAPI(float temperature) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("https://ir-on-sight.vercel.app/api/webhook");
    http.addHeader("Content-Type", "application/json");

    String jsonBody = "{\"temperature\":" + String(temperature, 2) + "}";
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
}

void onMessage(char *topic, uint8_t* msg, unsigned int msglen) {
  String message = "";
  for (int i = 0; i < msglen; i++) {
    message += (char)msg[i];
  }

  // Parse message: assuming "temperature,status" format, e.g., "25.3,ON"
  String tempStr = "";
  String statusStr = "";
  int commaIndex = message.indexOf(',');
  if (commaIndex > 0) {
    tempStr = message.substring(0, commaIndex);
    statusStr = message.substring(commaIndex + 1);
  } else {
    tempStr = message; // default if no comma
    statusStr = "UNKNOWN";
  }

  // Convert temperature to float and save globally
  lastTemperature = tempStr.toFloat();

  // Send message to UART (Serial)
  Serial.print("temperature:");
  Serial.print(lastTemperature, 2);
  Serial.print(",status:");
  Serial.println(statusStr);

  // DO NOT call API here
}

void onPresent(char *alias) {
  Serial.print("Device online: ");
  Serial.println(alias);
}

void onAbsent(char *alias) {
  Serial.print("Device offline: ");
  Serial.println(alias);
}

void onError(char *error) {
  Serial.print("NETPIE Error: ");
  Serial.println(error);
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

    // Send the last received temperature to API
    sendTemperatureToAPI(lastTemperature);
  }
}