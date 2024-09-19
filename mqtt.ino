#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

int ArduinoLED = 2;

// AHT20 start
#include <Adafruit_AHTX0.h>
Adafruit_AHTX0 aht;

int tempC; // To store the temperature in C
int tempF; // temp in F
int humidity; // To store the humidity

// ENS160 library
#include "ScioSense_ENS160.h" 
ScioSense_ENS160 ens160(ENS160_I2CADDR_1); // 0x53..ENS160+AHT21

// WiFi and MQTT settings
const char* ssid = "<WIFI-SSID>";
const char* password = "<WIFI-Password>";
const char* mqtt_server = "<MQTTServer>";
const char* mqtt_user = "<USER>>";
const char* mqtt_password = "<PASSWORD>";
const char* mqtt_topic = "sensor/data";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
    Serial.begin(9600);
    while (!Serial) {}
    Serial.println("------------------------------------------------------------");
    Serial.println("ENS160 - Digital air quality sensor");
    Serial.println();
    Serial.println("Sensor readout in standard mode");
    Serial.println();
    Serial.println("------------------------------------------------------------");
    delay(1000);

    Serial.print("ENS160...");
    ens160.begin();
    Serial.println(ens160.available() ? "done." : "failed!");

    if (ens160.available()) {
        // Print ENS160 versions
        Serial.print("\tRev: "); Serial.print(ens160.getMajorRev());
        Serial.print("."); Serial.print(ens160.getMinorRev());
        Serial.print("."); Serial.println(ens160.getBuild());
        Serial.print("\tStandard mode ");
        Serial.println(ens160.setMode(ENS160_OPMODE_STD) ? "done." : "failed!");
    }

    // AHT20 start
    Serial.println("Adafruit AHT10/AHT20 demo!");
    if (!aht.begin()) {
        Serial.println("Could not find AHT? Check wiring");
        while (1) delay(10);
    }
    Serial.println("AHT10 or AHT20 found");

    // Setup WiFi and MQTT
    setup_wifi();
    client.setServer(mqtt_server, 1883);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

     // Switch on LED for to confirm everything is okay.
    pinMode(ArduinoLED, OUTPUT);
    digitalWrite(ArduinoLED, LOW);

    ///// AHT20 start
    sensors_event_t humidity1, temp; 
    aht.getEvent(&humidity1, &temp); // Populate temp and humidity objects with fresh data
    tempC = (temp.temperature);
    tempF = (temp.temperature) * 1.8 + 32;
    humidity = (humidity1.relative_humidity);

    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.println(" degrees C");
    Serial.print("Temperature: ");
    Serial.print(tempF);
    Serial.println(" degrees F");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println("% rH");

    // Publish temperature and humidity to MQTT
    String payload = "Temperature: " + String(tempC) + " C, Humidity: " + String(humidity) + "%";
    client.publish(mqtt_topic, payload.c_str());

    delay(1000);
    ///// AHT20 end

    if (ens160.available()) {
        // Give values to Air Quality Sensor.
        ens160.set_envdata(tempC, humidity);
        ens160.measure(true);
        ens160.measureRaw(true);

        // Read AQI, TVOC, and eCO2 values
        int aqi = ens160.getAQI();
        int tvoc = ens160.getTVOC();
        int eco2 = ens160.geteCO2();

        Serial.print("AQI: "); Serial.print(aqi); Serial.print("\t");
        Serial.print("TVOC: "); Serial.print(tvoc); Serial.print("ppb\t");
        Serial.print("eCO2: "); Serial.print(eco2); Serial.println("ppm\t");

        // Publish AQI, TVOC, and eCO2 to MQTT
        String airQualityPayload = "AQI: " + String(aqi) + ", TVOC: " + String(tvoc) + " ppb, eCO2: " + String(eco2) + " ppm";
        client.publish(mqtt_topic, airQualityPayload.c_str());
    }

    delay(1000);
}

void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
            Serial.println("connected");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" Trying again in 5 seconds");
            delay(5000);
        }
    }
}
