#include <WiFi.h>
#include <MicroGear.h>

// -------- NETPIE credentials --------
#define APPID   "1d4c1ec7-9efe-4b3e-a9fd-903471a73f39"
#define KEY     "FYJpS15HcvRvF7KtCwrnqC2t7bLKsj2R"
#define SECRET  "z7jBNvj4JckL8Vxsu885jGC7EyKwi5xa"
#define ALIAS   "ESP32_NodeMCU"

// -------- Access Point settings --------
const char* ap_ssid = "ESP32_AP";
const char* ap_password = "12345678";

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

void setup() {
  Serial.begin(115200);
  delay(1000);

  // -------- Create WiFi Access Point --------
  Serial.println("Setting up Access Point...");
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ap_ssid, ap_password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("Access Point IP: ");
  Serial.println(IP);

  // -------- Connect to the internet via WiFi (optional) --------
  // If you want the ESP32 to connect to another WiFi for internet:
  // WiFi.begin("YourWiFiName", "YourWiFiPassword");
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }
  // Serial.println("Connected to Internet");

  // -------- Initialize NETPIE connection --------
  microgear.on(MESSAGE, onMessage);
  microgear.on(CONNECTED, onConnected);
  microgear.init(KEY, SECRET, ALIAS);

  Serial.println("Connecting to NETPIE...");
  microgear.connect(APPID);
}

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
    microgear.publish("/hello", "Hello from ESP32 Access Point!");
    Serial.println("Message sent to NETPIE");
  }
}
