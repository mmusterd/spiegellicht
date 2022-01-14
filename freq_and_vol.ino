/****************************************
  Example Sound Level Sketch for the
  Adafruit Microphone Amplifier
****************************************/
#include "Arduino.h"
#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#define FASTLED_ALLOW_INTERRUPTS 0
#include "FastLED.h"
#include "arduinoFFT.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <cstdint>
#include "volumes.h"

const int N_PIXELS = NUM_LEDS;
#define DATA_PIN D4

//fft
//
constexpr double SAMPLING_FREQUENCY = 5000.0; //Hz, must be less than 10000 due to ADC
unsigned int sample;
constexpr int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)


//smoothing
//
const int numReadings = 10;

int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average
enum _displaymode {_audio, _rainbow, _twocolor};
_displaymode displayMode = _twocolor;
enum _twofaceMode {_blend, _twoface};
_twofaceMode twocolorMode = _twoface;
int inputPin = A0;
CRGB StartColor = CRGB::Green;
CRGB EndColor = CRGB::Red;
//
//

int f = 0;
int val = 175;
CRGB leds[NUM_LEDS];
const char* ssid = "Luistervink";
const char* password = "Kletsen, dat is mijn ding";

ESP8266WebServer server(80);

// Serving Hello world
void getHelloWord() {
  server.send(200, "text/json", "{\"name\": \"Hello world\"}");
}
void sendSuccess(String command)
{
  Serial.println(command);
  server.send(200, F("text/html"),
              command);
}
// Define routing
void restServerRouting() {
  server.on("/", HTTP_GET, []() {
    server.send(200, F("text/html"),
                F("Welcome to the <strong>REST</strong> Web Server"));
  });
  server.on(F("/helloWorld"), HTTP_GET, getHelloWord);
  server.on(F("/rainbow"), HTTP_GET, []() {
    displayMode = _rainbow;
    sendSuccess("rainbow");
  });
  server.on(F("/audio"), HTTP_GET, []() {
    displayMode = _audio;
    sendSuccess("audio");
  });
  server.on(F("/twoface"), HTTP_GET, []() {
    displayMode = _twocolor;
    sendSuccess("twoface");
  });
  server.on(F("/blend"), HTTP_GET, []() {
    twocolorMode = _blend;
    sendSuccess("blend");
  });
  server.on(F("/hard"), HTTP_GET, []() {
    twocolorMode = _twoface;
    sendSuccess("hard");
  });
  server.on(F("/volume"), HTTP_GET, []() {
    String message = "volume\n";
    for (uint8_t i = 0; i < server.args(); i++) {
      message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    sendSuccess(message);
  });
}

// Manage not found URL
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

//fft
//
arduinoFFT FFT = arduinoFFT();
unsigned int sampling_period_us;
unsigned long microseconds;
Volumes volumes(3, 0); // three bars with 3*2 pixels overlap
struct HSV {
  int hue;
  int sat;
  int val;
};

double vReal[SAMPLES];
double vImag[SAMPLES];
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
HSV colorLED[150];
HSV black;
//
//
float bins[3];

void ReadAudioFFT()
{
  for (int i = 0; i < SAMPLES; i++)
  {
    microseconds = micros();    //Overflows after around 70 minutes!

    vReal[i] = analogRead(0);
    vImag[i] = 0;

    while (micros() < (microseconds + sampling_period_us)) {
    }
  }
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
  // vReal now contains the magnitudes; let's put them into three bins
  for (auto& bin : bins) {
    bin = 0.0;
  }
  size_t binIndex = 0;
  uint32_t binSize = (SAMPLES / 2) / 3;

  for (uint16_t index = 1; index < SAMPLES / 2; ++index) {
    if (index < binSize) binIndex = 0;
    else if (index < binSize * 2) binIndex = 1;
    else binIndex = 2;
    bins[binIndex] += vReal[index];
  }
  for (auto& bin : bins) {
    bin = bin / binSize;
  }
}

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

  randomSeed(98155);


  //fft
  //
  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));

  black.hue = 0;
  black.sat = 0;
  black.val = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  // Set server routing
  restServerRouting();
  // Set not found response
  server.onNotFound(handleNotFound);
  // Start server
  server.begin();
  Serial.println("HTTP server started");

  // Activate mDNS this is used to be able to connect to the server
  // with local DNS hostname spiegel.local
  if (MDNS.begin("spiegel")) {
    Serial.println("MDNS responder started");
  }
  displayMode = _twocolor;
}

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter)
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void loop()
{
  EVERY_N_MILLISECONDS(20) {
    server.handleClient();
    ReadAudioFFT();
    volumes.Update(0, max(0.0f, ((bins[0] / 1024) * NUM_LEDS) - 6));
    volumes.Update(1, max(0.0f, ((bins[1] / 1024) * NUM_LEDS)));
    volumes.Update(2, max(0.0f, ((bins[2] / 700) * NUM_LEDS)));
    switch (displayMode) {
      case _audio:
        for (int i = 0; i < NUM_LEDS; ++i) {
          leds[i] = volumes.GetAt(i);
        }
        break;
      case _rainbow:
        rainbowWithGlitter();
        gHue++;
        break;
      case _twocolor:
        if (twocolorMode == _blend) {
          fill_gradient_RGB(leds, NUM_LEDS, StartColor, EndColor);
        } else {
          for (int i = 0; i < NUM_LEDS; ++i) {
            leds[i] = i < NUM_LEDS / 2 ? StartColor : EndColor;
          }
        }
      default:
        break;
    }
    FastLED.show();
  }
}
