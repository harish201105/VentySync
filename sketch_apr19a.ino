#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "YourWiFiName";           // Replace with your WiFi
const char* password = "YourWiFiPassword";   // Replace with your password

const String readUrl = "https://api.thingspeak.com/channels/2850939/feeds.json?api_key=UYWGFVRNBX44CWD0&results=1";
const String writeUrl = "https://api.thingspeak.com/update?api_key=AU9JMBVBXZOC8QJX&field1=";

const int switchPin = 14;    // Physical switch
const int relayPin = 4;      // Relay control

void setup() {
  Serial.begin(115200);
  pinMode(switchPin, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

void loop() {
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 5000) {
    lastCheck = millis();

    int apiValue = getStatusFromThingSpeak();
    int switchValue = digitalRead(switchPin) == LOW ? 1 : 0;
    int finalState = (switchValue == 1 || apiValue == 1) ? 1 : 0;

    digitalWrite(relayPin, finalState);
    Serial.println(finalState ? "Valve ON" : "Valve OFF");

    sendStatusToThingSpeak(finalState);
  }
}

int getStatusFromThingSpeak() {
  HTTPClient http;
  http.begin(readUrl);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    String val = doc["feeds"][0]["field1"];
    return val.toInt();
  } else {
    Serial.println("Failed to fetch from ThingSpeak");
    return 0;
  }
}

void sendStatusToThingSpeak(int value) {
  HTTPClient http;
  http.begin(writeUrl + String(value));
  int httpCode = http.GET();
  if (httpCode == 200) {
    Serial.println("Status sent to ThingSpeak");
  } else {
    Serial.println("Failed to send status");
  }
  http.end();
}