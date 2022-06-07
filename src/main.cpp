#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
/* */
#include <ArduinoJson.h>

/* timer */
#include <Ticker.h>

ESP8266WiFiMulti WiFiMulti;

#define LED 2
#define WIFI_SSID "DIGISOL"
#define WIFI_KEY "12345699"
#define SENSOR 14

bool ledState = false;
Ticker ticker;

/* state variable */
volatile int mode = 0;       // default is reading mode
volatile int processing = 0; // lock variable

/* main counter variable */
volatile int counter = 0;

void uploadDataStateTrigger()
{
  if (processing == 1)
  {
    return;
  }
  Serial.println("Data uploading init...");
  mode = 1;
}

unsigned long t_counter = 0;
unsigned long c_counter = 0;

IRAM_ATTR void detectsMovement()
{
  // c_counter = millis();
  // if (c_counter - t_counter < 100)
  // {
  //   return;
  // }

  // c_counter = 0;
  // t_counter = 0;
  ++counter;
  // Serial.println("MOTION DETECTED!!!");
  // t_counter = millis();
}

void setup()
{

  Serial.begin(115200);

  pinMode(SENSOR, INPUT);
  attachInterrupt(digitalPinToInterrupt(SENSOR), detectsMovement, FALLING);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, 0);

  ticker.attach(20, uploadDataStateTrigger);

  for (uint8_t t = 4; t > 0; t--)
  {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WIFI_SSID, WIFI_KEY);
}

void loop()
{
  /* Data upload State */
  if (mode == 1)
  {
    digitalWrite(LED, 0);
    processing = 1; /* turning on lock */
    int counter_copy = counter;
    if ((WiFiMulti.run() == WL_CONNECTED))
    {
      WiFiClient client;
      HTTPClient http;
      String URL = "http://192.168.2.5:5000/api/logger/test";

      http.begin(client, URL);
      http.addHeader("Content-Type", "application/json");

      int httpCode = http.POST("{\"counter\":" + String(counter_copy) + "}");

      if (httpCode > 0)
      {
        if (httpCode == HTTP_CODE_OK)
        {
          // const String &payload = http.getString();
          // Serial.println("received payload:\n<<");
          // Serial.println(payload);
          // Serial.println(">>");

          Serial.println("Counter value uploaded !");

          /* reset counter variable */
          counter = 0;
        }
      }
      else
      {
        Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    }
    else
    {
      Serial.printf("Wifi Not Connected !\n");
    }

    /* reset mode */
    mode = 0;
    processing = 0; // release lock
    delay(1000);
    digitalWrite(LED, 1);
  }
}