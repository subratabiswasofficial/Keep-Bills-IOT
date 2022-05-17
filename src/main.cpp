#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
/* */
#include <ArduinoJson.h>

ESP8266WiFiMulti WiFiMulti;
void setup()
{
  Serial.begin(115200);
  for (uint8_t t = 4; t > 0; t--)
  {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("DIGISOL", "12345699");
}

void loop()
{
  if ((WiFiMulti.run() == WL_CONNECTED))
  {
    WiFiClient client;
    HTTPClient http;

    http.begin(client, "http://jsonplaceholder.typicode.com/todos/1");
    // http.addHeader("Content-Type", "application/json");

    // int httpCode = http.POST("{\"name\":\"subrata biswas\",\"age\":12}");
    int httpCode = http.GET();

    if (httpCode > 0)
    {
      if (httpCode == HTTP_CODE_OK)
      {
        const String &payload = http.getString();
        Serial.println("received payload:\n<<");
        Serial.println(payload);
        Serial.println(">>");
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

  // DynamicJsonDocument doc(1024);
  // deserializeJson(doc, F("{\"sensor\":\"gps\",\"time\":1351824120,"
  //                        "\"data\":[48.756080,2.302038]}"));
  // JsonObject obj = doc.as<JsonObject>();
  // long time = obj[F("time")];
  // Serial.print("Json example ");
  // Serial.print(time);
  // Serial.println();
  delay(3000);
}