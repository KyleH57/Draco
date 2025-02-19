#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "Constellation.h"

const char *host = "esp32";

// variabls for blinking an LED with Millis
const int led = 2;                // ESP32 Pin to which onboard LED is connected
unsigned long previousMillis = 0; // will store last time LED was updated
const long interval = 500;        // interval at which to blink (milliseconds)
int ledState = LOW;               // ledState used to set the LED

WebServer server(80);
Constellation *myConstellation;

WiFiManager wifiManager;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -7 * 60 * 60);

WiFiClient client;
const char *ledServer = "192.168.0.113";
const uint16_t port = 4551;

bool connectedToLedServer = false;

const uint16_t DEBOUNCE_TIME = 200; // in milliseconds

void IRAM_ATTR left_isr_handler_press();
#define GPIO_NUM 0
volatile unsigned long buttonPressTime = 0;
volatile unsigned long buttonReleaseTime = 0;
volatile unsigned long elapsedTime = 0;

volatile int brightness = 0;
volatile int right_button_presses = 3;

#define RIGHT_BUTTON 26
void IRAM_ATTR right_isr_handler();

bool auto_on = false;
bool auto_turned_off = false;
int prevDay = -1;

/* Style */
String style =
    "<style>#file-input,input{width:100%;height:44px;border-radius:4px;margin:10px auto;font-size:15px}"
    "input{background:#f1f1f1;border:0;padding:0 15px}body{background:#3498db;font-family:sans-serif;font-size:14px;color:#777}"
    "#file-input{padding:0;border:1px solid #ddd;line-height:44px;text-align:left;display:block;cursor:pointer}"
    "#bar,#prgbar{background-color:#f1f1f1;border-radius:10px}#bar{background-color:#3498db;width:0%;height:10px}"
    "form{background:#fff;max-width:258px;margin:75px auto;padding:30px;border-radius:5px;text-align:center}"
    ".btn{background:#3498db;color:#fff;cursor:pointer}</style>";

/* Server Index Page */
String serverIndex =
    "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
    "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
    "<input type='file' name='update' id='file' onchange='sub(this)' style=display:none>"
    "<label id='file-input' for='file'>   Choose file...</label>"
    "<input type='submit' class=btn value='Update'>"
    "<br><br>"
    "<div id='prg'></div>"
    "<br><div id='prgbar'><div id='bar'></div></div><br></form>"
    "<script>"
    "function sub(obj){"
    "var fileName = obj.value.split('\\\\');"
    "document.getElementById('file-input').innerHTML = '   '+ fileName[fileName.length-1];"
    "};"
    "$('form').submit(function(e){"
    "e.preventDefault();"
    "var form = $('#upload_form')[0];"
    "var data = new FormData(form);"
    "$.ajax({"
    "url: '/update',"
    "type: 'POST',"
    "data: data,"
    "contentType: false,"
    "processData:false,"
    "xhr: function() {"
    "var xhr = new window.XMLHttpRequest();"
    "xhr.upload.addEventListener('progress', function(evt) {"
    "if (evt.lengthComputable) {"
    "var per = evt.loaded / evt.total;"
    "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
    "$('#bar').css('width',Math.round(per*100) + '%');"
    "}"
    "}, false);"
    "return xhr;"
    "},"
    "success:function(d, s) {"
    "console.log('success!') "
    "},"
    "error: function (a, b, c) {"
    "}"
    "});"
    "});"
    "</script>" +
    style;

/* setup function */
void setup(void)
{
  // Initialize constellation
  std::string angles = "60r, 180, 120, 60, 0r, 120, 60, 0, -60, -120, -60, 0, 60, 120, 180r";
  int num_leds_segment = 15;  // Example value. Adjust as needed.
  float spacing = 15.0f;      // Example value. Adjust as needed.
  float edge_spacing = 13.0f; // Example value. Adjust as needed.
  // float brightness = 0.5f;    // Dead code???

  myConstellation = new Constellation(angles, num_leds_segment, spacing, edge_spacing);
  // myConstellation is now initialized and can be used

  pinMode(GPIO_NUM, INPUT_PULLUP);                                                   // set GPIO0 as input, enable internal pullup resistor
  attachInterrupt(digitalPinToInterrupt(GPIO_NUM), left_isr_handler_press, FALLING); // attach the interrupt
  // attachInterrupt(digitalPinToInterrupt(GPIO_NUM), left_isr_handler_release, RISING);

  pinMode(RIGHT_BUTTON, INPUT_PULLUP);                                              // set GPIO0 as input, enable internal pullup resistor
  attachInterrupt(digitalPinToInterrupt(RIGHT_BUTTON), right_isr_handler, FALLING); // attach the interrupt

  // wifiManager.resetSettings();

  // Set AP timeout to 180 seconds. Can be increased if needed.
  wifiManager.setConfigPortalTimeout(60);

  // Starts the AP (Access Point) if the ESP32 can't connect to the Wi-Fi network
  if (!wifiManager.autoConnect("hot_tub_stream_prod", "")) // no password for now
  {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    // Reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  }

  pinMode(led, OUTPUT);

  Serial.begin(115200);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  // Serial.println("");
  Serial.print("Connected to wifi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /*use mdns for host name resolution*/
  if (!MDNS.begin(host))
  { // http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1)
    {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []()
            {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex); });
  /*handling uploading firmware file */
  server.on(
      "/update", HTTP_POST, []()
      {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart(); },
      []()
      {
        HTTPUpload &upload = server.upload();
        if (upload.status == UPLOAD_FILE_START)
        {
          Serial.printf("Update: %s\n", upload.filename.c_str());
          if (!Update.begin(UPDATE_SIZE_UNKNOWN))
          { // start with max available size
            Update.printError(Serial);
          }
        }
        else if (upload.status == UPLOAD_FILE_WRITE)
        {
          /* flashing firmware to ESP*/
          if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
          {
            Update.printError(Serial);
          }
        }
        else if (upload.status == UPLOAD_FILE_END)
        {
          if (Update.end(true))
          { // true to set the size to the current progress
            Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
          }
          else
          {
            Update.printError(Serial);
          }
        }
      });
  server.begin();

  timeClient.begin(); // Initialize the NTP client

  // // connect to led server
  // if (client.connect(ledServer, port))
  // {
  //   Serial.println("Connected to LED server");
  //   connectedToLedServer = true;
  // }
  // else
  // {
  //   Serial.println("Connection to LED server failed");
  // }
}

bool wifi_reset_flag = false;
bool left_button_pressed = false;
bool right_button_pressed = false;

void loop(void)
{
  unsigned long elapsed_time = millis();

  myConstellation->run(elapsed_time, right_button_presses, brightness);

  timeClient.update(); // Update the NTP client

  int hours = timeClient.getHours();
  int minutes = timeClient.getMinutes();
  int seconds = timeClient.getSeconds();
  int day = timeClient.getDay();

  if (prevDay != day)
  { // Day has changed, reset the flag
    prevDay = day;
    auto_on = false;
    auto_turned_off = false;
  }

  if (!auto_on && brightness == 0 && hours == 18 && minutes == 0)
  {
    // auto reboot
    const unsigned long ROLLOVER_THRESHOLD = 360000000UL;
    if (millis() >= ROLLOVER_THRESHOLD && seconds == 0)
    {
      Serial.println("Millis approaching rollover - Rebooting!");
      ESP.restart(); // or ESP.reset() in older cores
    }

    Serial.println("It's 6PM!");

    brightness = 51;

    auto_on = true; // Set the flag to true to prevent this action from happening again today
  }

  if (!auto_turned_off && brightness > 0 && hours == 23 && minutes == 59)
  {
    Serial.println("It's 11:59PM!");
    // Add the code you want to execute at 11:59PM here.
    // ...
    brightness = 0;

    auto_turned_off = true; // Set the flag to true to prevent this action from happening again today
  }

  server.handleClient(); // Handle things like web requests

  if (wifi_reset_flag)
  {
    Serial.println("Resetting wifi");
    myConstellation->run(0, 1, 77); // show all white
    wifiManager.resetSettings();
    ESP.restart();
  }

  if (left_button_pressed) // adjust brightness
  {
    left_button_pressed = false;
    brightness += 51;
    Serial.println(brightness);

    if (brightness > 255)
    {
      brightness = 0;
    }
  }

  if (right_button_pressed) // cycle the patterns
  {
    right_button_pressed = false;
    right_button_presses += 1;
    Serial.println(right_button_presses);

    if (right_button_presses > 4)
    {
      right_button_presses = 0;
    }
  }

  // read gpio0
  int reset_time = 0;
  while (digitalRead(GPIO_NUM) == LOW && digitalRead(RIGHT_BUTTON) == LOW)
  {
    delay(10);
    myConstellation->reset_Animation(reset_time);
    reset_time++;
    if (reset_time > 225)
    {
      wifi_reset_flag = true;
      break;
    }
  }

  // loop to blink the debug without delay
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    if (connectedToLedServer)
    {
      client.println("Hello from ESP32!");

      // Read and print the response
      while (client.available())
      {
        String line = client.readStringUntil('\n');
        Serial.println(line);
      }
      // save the last time you blinked the LED
      previousMillis = currentMillis;
      // if the LED is off turn it on and vice-versa:
      ledState = not(ledState);
      // set the LED with the ledState of the variable:
      digitalWrite(led, ledState);
    }
  }
}

void IRAM_ATTR left_isr_handler_press() // IO0
{
  unsigned long currentTime = millis();
  if (currentTime - buttonPressTime > DEBOUNCE_TIME)
  {
    buttonPressTime = currentTime;
    left_button_pressed = true;
  }
}

void IRAM_ATTR right_isr_handler() // IO26
{
  unsigned long currentTime = millis();
  if (currentTime - buttonPressTime > DEBOUNCE_TIME)
  {
    buttonPressTime = currentTime;
    right_button_pressed = true;
  }
}