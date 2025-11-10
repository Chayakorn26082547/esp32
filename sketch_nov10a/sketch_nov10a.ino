#include <WiFi.h>
#include <MicroGear.h>

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

// -------- Event Handlers --------
void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.println(">>> Connected to NETPIE 2020");
  microgear.setAlias(ALIAS);
}

void onMessage(char *topic, uint8_t* msg, unsigned int msglen) {
  Serial.print("Incoming message: ");
  for (int i = 0; i < msglen; i++) {
    Serial.print((char)msg[i]);
  }
  Serial.println();
}

// -------- WiFi Setup --------
void setupWiFi() {
  Serial.println();
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

// -------- Arduino Setup --------
void setup() {
  Serial.begin(115200);
  delay(1000);

  setupWiFi();

  // NETPIE event bindings
  microgear.on(MESSAGE, onMessage);
  microgear.on(CONNECTED, onConnected);

  // Initialize MicroGear and connect to NETPIE
  microgear.init(KEY, SECRET, ALIAS);
  Serial.println("Connecting to NETPIE...");
  microgear.connect(APPID);
}

// -------- Main Loop --------
void loop() {
  if (microgear.connected()) {
    microgear.loop();
  } else {
    Serial.println("Reconnecting to NETPIE...");
    microgear.connect(APPID);
  }

  // Example: publish every 5 seconds
  static unsigned long lastSend = 0;
  if (millis() - lastSend > 5000) {
    lastSend = millis();
    microgear.publish("/hello", "Hello from ESP32!");
    Serial.println("Message sent to NETPIE");
  }
}