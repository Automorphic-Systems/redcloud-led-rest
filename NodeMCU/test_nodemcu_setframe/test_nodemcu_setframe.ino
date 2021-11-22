#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include <stdio.h>

/* Config file */
#define LEDHOST_CONFIG "/ledhost_config.json"
#define DEFAULT_WIFI_TIMEOUT 90000
#define LED_PIN 7
#define NUM_LEDS 288
#define FRAME_INTERVAL 64
#define HSV_DEFAULT_VALUE 160

StaticJsonDocument<1024> configFile;
/*
/* SetFrame Test for NodeHost_MCU */
/*



/* WiFi Credentials */
const char* ssid;
const char* password;
bool is_conn;

/* HTTP Setup */
ESP8266WebServer server(8000);

/* LED and heatmap values*/
CRGB leds[NUM_LEDS];

/* Timing */
char logbuffer [80];
bool cycle;
int logsize;
unsigned long currTime, prevTime, startTime, cycleTime;

/* Setup  */
void setup() {
  
  /* Set pins, clear UART */  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(D4, OUTPUT);
  delay(5000);
  Serial.begin(115200);
  delay(500);
  Serial.flush();
  while ( Serial.available() ) Serial.read(); 
  
  Serial.println("Initializing parameters, activating LEDs");
  is_conn = false;
  cycle = 0;
  
  /* Set LED strip */
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);

  testLEDs();

  /* Verify config file */
  testLoadConfig();

  /* Verify wi-fi connectivity */  
  testWifiSetup();

  if (is_conn) {
    /* Verify web host setup */
    testWebHostSetup();
  }
}

/* Loop */
void loop() {    
  currTime = millis();
  server.handleClient();

  if (currTime - prevTime > FRAME_INTERVAL) {
     FastLED.show();       
     prevTime = currTime;      
  } 

  if (cycle) {    
    cycleTime = millis();
    cycle = 0;
  }  
}

void testLEDs() {
  Serial.println("Testing leds...");
  for (int i=0;i<NUM_LEDS;i++) {
   leds[i]=CHSV(128,255,255);
  }

  FastLED.show();
  delay(5000);
  FastLED.clearData();
  FastLED.show();
}

void testWifiSetup() {
    unsigned long wifi_time = millis();
    Serial.print("Configuring access point...");            
    WiFi.begin(ssid, password);
    bool is_timeout = false;

    while ((WiFi.status() != WL_CONNECTED) && (!is_timeout)) {
        if (millis() - wifi_time < DEFAULT_WIFI_TIMEOUT) {
            delay(1000);
            Serial.print(".");
        } else {
            is_timeout = true;
        }
    }   
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi connected \n\r\ IP address: ");
        Serial.println(WiFi.localIP());  
        is_conn = true;
    } else {
        Serial.println("Wifi not connected.\n Web server not enabled.\n Using default settings.");
    } 
}

void testWebHostSetup() {
    server.on("/", handleRoot);
    server.on("/testget", testGet);
    server.on("/testsetframe", HTTP_POST, testSetFrame);
    server.on("/led_on", handleLedOn);
    server.on("/led_off", handleLedOff);
    server.begin();
    Serial.println ( "HTTP server started" );  
}

/* Web Server Handlers */
void handleRoot() {
 server.send(200, "text/html", "<h2>Node MCU LED Host</h2><br/>IP Address - " + WiFi.localIP().toString());
}

void handleLedOn() {
    Serial.println("Turning on board LED on..");
    digitalWrite(LED_BUILTIN, 0);
    server.send(200, "text/plain","{ result: 'led on' }");
}

void handleLedOff() {
    Serial.println("Turning on board LED off..");
    digitalWrite(LED_BUILTIN, 1);
    server.send(200, "text/plain","{ result: 'led off' }");
}

void testGet() {   
    Serial.println("GET request received...");
    server.send(200, "text/plain","{ result: 'get' }");
}

void testSetFrame() {
//    Serial.println("SetFrame test...");
//    Serial.print("Arguments: ");
//    Serial.print(server.args());
//    Serial.print(", Array size: ");
//    Serial.println(sizeof(server.arg(0)));
  
    for(int x=0; x<NUM_LEDS; x++) {
      leds[x]=CHSV(server.arg(0)[x], 255, 255);
      
      //Serial.print(server.arg(0)[x], HEX);
      //Serial.print(",");
    }    

    cycle = 1;
    server.send(200, "text/plain", "OK");
}
void testLoadConfig() {
  
    if (SPIFFS.begin()) {
        Serial.println("Reading from file system..");
    
        File f = SPIFFS.open(LEDHOST_CONFIG, "r");
    
        if (f && f.size()) {
            Serial.println("Opening config..");
            size_t filesize = f.size();
            
            char jsonData[filesize+1];
            f.read((uint8_t *)jsonData, sizeof(jsonData));
            f.close();
            jsonData[filesize]='\0';
            
            Serial.println("Output of config file: ");
            Serial.println(jsonData);
            
            DeserializationError error = deserializeJson(configFile, jsonData);

            if (error) {
                Serial.println("Failure to parse json data");
                return;
            }

            ssid = configFile["wifi_ssid"];
            password = configFile["wifi_pwd"]; 

            Serial.print("SSID: ");
            Serial.println(ssid);
            Serial.print("Password: ");    
            Serial.println(password);            
        }    
    }
}
