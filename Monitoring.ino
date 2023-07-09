#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

#define DHT_PIN 4
#define DHT_TYPE DHT22

#define BMP_SDA_PIN 19
#define BMP_SCL_PIN 23

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_BMP280 bmp;

const char* ssid = "h";
const char* password = "11032023";
const char* server = "api.thingspeak.com";
String myAPIkey = "7JENOBQ9M9N6F9XR";

unsigned long previousMillis = 0;
const long interval = 180000; // Interval waktu dalam milidetik (3 menit = 3 x 60 x 1000 ms)

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println(F("BMP280 test"));
  unsigned status;
  lcd.init();
  lcd.backlight();
  status = bmp.begin(0x76);
  if (!status) {
    Serial.println(F("Error bang, coba lagi "));
    Serial.print("SensorID was: 0x");
    Serial.println(bmp.sensorID(), 16);
    Serial.print(" ID of 0xFF probably means bad addreas bg, ya gitulah \n");
    Serial.print(" ID of 0x76");
    while (1) delay(10);
  }

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);

  dht.begin();
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Suhu:");
  lcd.setCursor(0, 1);
  lcd.print("Kelembapan:");
  lcd.setCursor(0, 0);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  unsigned long currentMillis = millis();

  // Perekaman data setiap tiga menit sekali
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    float heatIndex = dht.computeHeatIndex(temperature, humidity, false);
    float pressure = bmp.readPressure() / 100.0;

    Serial.print("T: ");
    Serial.print(temperature);
    Serial.print(" °C\tH: ");
    Serial.print(humidity);
    Serial.print(" %\tHi: ");
    Serial.print(heatIndex);
    Serial.println(" °C");
    Serial.print("P: ");
    Serial.print(pressure);
    Serial.println(" hPa");

    lcd.setCursor(6, 0);
    lcd.print(temperature);
    lcd.setCursor(6, 1);
    lcd.print(humidity);
    lcd.setCursor(12, 0);
    lcd.print(pressure);
    lcd.setCursor(0, 1);
    lcd.print("Tekanan:");

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      String url = "http://";
      url += server;
      url += "/update?api_key=";
      url += myAPIkey;
      url += "&field1=";
      url += String(temperature);
      url += "&field2=";
      url += String(humidity);
      url += "&field3=";
      url += String(pressure);

      http.begin(url);
      int httpCode = http.GET();
      if (httpCode > 0) {
        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);
      }
      else {
        Serial.println("Error on HTTP request");
      }
      http.end();
    }
    else {
      Serial.println("WiFi disconnected");
    }
  }
}