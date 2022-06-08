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
#define SENSOR 14 // D5

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
  Serial.println("BLINK !!!");
  ++counter;
  // t_counter = millis();
}

const int httpsPort = 443; // HTTPS= 443 and HTTP = 80

// SHA1 finger print of certificate use web browser to view and copy
const char fingerprint[] PROGMEM = "b2 46 ef e7 46 23 4a 97 72 fa 97 41 8a 7a de c9 24 18 36 36";
const char *host = "keepbills.shop";
const char *uri = "/api/logger/log";

void setup()
{

  Serial.begin(115200);

  pinMode(SENSOR, INPUT);
  attachInterrupt(digitalPinToInterrupt(SENSOR), detectsMovement, FALLING);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, 0);

  ticker.attach(30, uploadDataStateTrigger);

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
      WiFiClientSecure httpsClient;
      httpsClient.setFingerprint(fingerprint);
      httpsClient.setTimeout(15000); // 15 Seconds
      delay(1000);

      int r = 0; // retry counter
      while ((!httpsClient.connect(host, httpsPort)) && (r < 30))
      {
        delay(100);
        Serial.print(".");
        r++;
      }

      if (r == 30)
      {
        Serial.println("Connection failed");
      }

      char body[40];
      int x = counter_copy;
      sprintf(body, "{\"counter\": %d}", x);
      char postStr[40];
      sprintf(postStr, "POST %s HTTP/1.1", uri);
      httpsClient.println(postStr);
      httpsClient.print("Host: ");
      httpsClient.println(host);
      httpsClient.println("Content-Type: application/json");
      httpsClient.print("Content-Length: ");
      httpsClient.println(strlen(body));
      httpsClient.println(); // extra `\r\n` to separate the http header and http body
      httpsClient.println(body);

      while (httpsClient.connected())
      {
        String line = httpsClient.readStringUntil('\n');
        if (line == "\r")
        {
          Serial.println("Data Uploaded !");
          counter -= counter_copy; // reset counter
          break;
        }
      }

      /*
      *
      // if I have to print output
      Serial.println("reply was:");
      Serial.println("==========");
      String line;
      while (httpsClient.available())
      {
        line = httpsClient.readStringUntil('\n'); // Read Line by Line
        Serial.println(line);                     // Print response
      }
      Serial.println("==========");
      Serial.println("closing connection");
      delay(2000); // POST Data at every 2 seconds
      *
      */
    }
    else
    {
      Serial.printf("Wifi Not Connected !\n");
    }

    // // if the connection is HTTP
    // if ((WiFiMulti.run() == WL_CONNECTED))
    // {
    //   // WiFiClient client;
    //   WiFiClientSecure client;
    //   HTTPClient http;

    //   // String URL = "http://192.168.2.5:5000/api/logger/log";
    //   String URL = "https://jsonplaceholder.typicode.com/todos/1";

    //   http.begin(client, URL);
    //   // http.addHeader("Content-Type", "application/json");

    //   // int httpCode = http.POST("{\"counter\":" + String(counter_copy) + "}");
    //   int httpCode = http.GET();

    //   if (httpCode > 0)
    //   {
    //     if (httpCode == HTTP_CODE_OK)
    //     {
    //       const String &payload = http.getString();
    //       Serial.println("received payload:\n<<");
    //       Serial.println(payload);
    //       Serial.println(">>");

    //       Serial.println("Counter value uploaded !");

    //       /* reset counter variable */
    //       counter = 0;
    //     }
    //   }
    //   else
    //   {
    //     Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    //   }

    //   http.end();
    // }
    // else
    // {
    //   Serial.printf("Wifi Not Connected !\n");
    // }

    /* reset mode */
    mode = 0;
    processing = 0; // release lock
    delay(1000);
    digitalWrite(LED, 1);
  }
}