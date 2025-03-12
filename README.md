## Description of the Code
This Arduino program, designed for the ESP8266 WiFi module, reads soil moisture data and fetches weather forecasts from OpenWeatherMap. It then uses this data to decide whether irrigation is necessary and logs the information to ThingSpeak for remote monitoring.

## Key Features:
<pre>
1. Connects to WiFi using provided SSID and password.
2.Fetches weather data from OpenWeatherMap API to check if rain is expected.
3.Reads soil moisture using a digital sensor.
4.Controls LED indicators to show soil condition:
.Green LED (D2): Soil moisture is good / Rain expected.
.Red LED (D3): Soil is too dry, irrigation needed.
5.Sends data to ThingSpeak, a cloud-based IoT platform for monitoring.
</pre>
## Breakdown of Code Sections
### 1. Libraries and WiFi Credentials
``` c
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
```
#### ESP8266WiFi.h: Handles WiFi connection.
#### ArduinoJson.h: Parses JSON responses from OpenWeatherMap API.
### 3. WiFi and API Credentials
``` c
const char* ssid = "Galaxy S23 E0D1";
const char* password = "11223344";

const char* apiKey = "de986ef8e78d6600fd0dcc661f7672c9";
const char* city = "Rabat";
const char* country = "MA"; 
const char* server = "api.openweathermap.org";

const char* thingSpeakServer = "api.thingspeak.com";
const String writeAPIKey = "KUAGHCLIPIV7SHG7";
```
#### Stores WiFi credentials and API keys for OpenWeatherMap and ThingSpeak.
### 3. Pin Definitions
``` c
#define sensorPower D7
#define sensorPin D8
#define greenLedPin D2
#define redLedPin D3
```
#### D7: Powers the soil moisture sensor.
#### D8: Reads soil moisture (1 = dry, 0 = moist).
#### D2/D3: Green and Red LEDs to indicate moisture level.
### 4. WiFi Connection in setup()
``` c
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }
    Serial.println("\nConnected to Wi-Fi!");
```
#### The ESP8266 connects to WiFi and prints a status update.
### 5. Main Logic in loop()
``` c
 bool rainExpected = checkRainForecast();
 int val = readSensor()
```
#### Calls previsionPluie() to check if it will rain.
#### Calls readSensor() to check soil moisture.
#### Moisture and Rain Decision
``` c
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
```
##### If rain is expected, no need to water; green LED is ON.
##### If soil is dry, red LED turns ON to indicate irrigation is needed.
### 6. Reading Soil Moisture
``` c
int readSensor() {
    digitalWrite(sensorPower, HIGH);
    delay(10);
    int val = digitalRead(sensorPin);
    digitalWrite(sensorPower, LOW);
    return val;
}
```
#### Activates the sensor, reads its output, then powers it off to save energy.
### 7. Fetching Weather Forecast
``` c
bool checkRainForecast() {
    if (client.connect(server, 80)) {
      String url = "/data/2.5/forecast?q=" + String(city) + "," + String(country) + "&appid=" + String(apiKey);
``` 
#### Sends a GET request to OpenWeatherMap to fetch a 5-day forecast.
#### Parses the JSON response to check for "rain" in the forecast.
``` c
for (JsonObject forecast : doc["list"].as<JsonArray>()) {
    if (forecast.containsKey("rain")) {
        return true; 
    }
}
```
#### If rain is found, returns true; otherwise, false.
### 8. Sending Data to ThingSpeak
``` c
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
```
#### Sends moisture and rain prediction data to ThingSpeak for monitoring.
## Summary of Functionality
1.Connects to WiFi
2.Fetches weather forecast (rain prediction)
3.Reads soil moisture sensor
4.Controls LEDs to indicate whether watering is needed
5.Sends data to ThingSpeak for remote monitoring
