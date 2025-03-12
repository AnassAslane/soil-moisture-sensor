#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

const char* ssid = "Galaxy S23 E0D1";
const char* password = "11223344";

const char* apiKey = "de986ef8e78d6600fd0dcc661f7672c9";
const char* city = "Rabat";
const char* country = "MA"; 
const char* server = "api.openweathermap.org";

const char* thingSpeakServer = "api.thingspeak.com";
const String writeAPIKey = "KUAGHCLIPIV7SHG7";

#define sensorPower D7
#define sensorPin D8
#define greenLedPin D2
#define redLedPin D3

WiFiClient client;

void setup() {
    pinMode(sensorPower, OUTPUT);
    pinMode(sensorPin, INPUT);
    pinMode(greenLedPin, OUTPUT);
    pinMode(redLedPin, OUTPUT);

    digitalWrite(sensorPower, LOW);
    Serial.begin(9600);

    Serial.print("Connecting to Wi-Fi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }
    Serial.println("\nConnected to Wi-Fi!");
}

void loop() {
    // Check weather forecast
    bool rainExpected = checkRainForecast();

    // Read soil moisture
    int val = readSensor();
    Serial.print("Digital output: ");
    Serial.println(val);

    if (rainExpected) {
      Serial.println("Rain expected tomorrow: No need to water.");
      digitalWrite(greenLedPin, HIGH); // Indicate good status
      digitalWrite(redLedPin, LOW);
    } else {
      if (val) {
        Serial.println("Soil too dry: Time to water!");
        digitalWrite(greenLedPin, LOW);
        digitalWrite(redLedPin, HIGH);
      } else {
        Serial.println("Soil moisture is perfect.");
        digitalWrite(greenLedPin, HIGH);
        digitalWrite(redLedPin, LOW);
      }
    }

    // Send data to ThingSpeak
    sendDataToThingSpeak(val, rainExpected);

    delay(6000); // Wait for 1 minute before the next check
}

int readSensor() {
    digitalWrite(sensorPower, HIGH);
    delay(10);
    int val = digitalRead(sensorPin);
    digitalWrite(sensorPower, LOW);
    return val;
}

bool checkRainForecast() {
    if (client.connect(server, 80)) {
      String url = "/data/2.5/forecast?q=" + String(city) + "," + String(country) + "&appid=" + String(apiKey);
      Serial.print("Requesting URL: ");
      Serial.println(url);

      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                  "Host: " + server + "\r\n" +
                  "Connection: close\r\n\r\n");

      while (!client.available()) {
        delay(100);
      }

      while (client.available()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
          break;
        }
      }

      String payload = client.readString();
      client.stop();

      DynamicJsonDocument doc(2048);
      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
        Serial.print("JSON deserialization error: ");
        Serial.println(error.c_str());
        return false;
      }

      for (JsonObject forecast : doc["list"].as<JsonArray>()) {
        if (forecast.containsKey("rain")) {
          return true; 
        }
      }

      return false; 
    } else {
      Serial.println("Failed to connect to server");
      return false;
    }
}

void sendDataToThingSpeak(int moisture, bool rain) {
    if (client.connect(thingSpeakServer, 80)) {
      String data = "field1=" + String(moisture) + "&field2=" + String(rain ? 1 : 0);
      client.print(String("POST /update HTTP/1.1\r\n") +
                  "Host: " + thingSpeakServer + "\r\n" +
                  "Connection: close\r\n" +
                  "X-THINGSPEAKAPIKEY: " + writeAPIKey + "\r\n" +
                  "Content-Type: application/x-www-form-urlencoded\r\n" +
                  "Content-Length: " + data.length() + "\r\n\r\n" +
                  data);
      client.stop();
      Serial.println("Data sent to ThingSpeak");
    } else {
      Serial.println("Failed to connect to ThingSpeak");
    }
}
