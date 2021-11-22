#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <ArduinoJson.h>

/* Config file */
#define LEDHOST_CONFIG "/ledhost_config.json"
#define DEFAULT_WIFI_TIMEOUT 90000

StaticJsonDocument<1024> configFile;

/* WiFi Credentials */
const char* ssid;
const char* password;
bool is_conn;

/* HTTP Setup */
ESP8266WebServer server(8000);


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
  is_conn = false;

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
    server.handleClient();
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
    server.on("/testpost", HTTP_POST, testPost);
    server.on("/testbinarypost", HTTP_POST, testBinaryPost);
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

void testPost() {   
    Serial.println("Text post detected...");
    
    String response = "{ result: '";
    response += server.arg("test");
    response += "'}";
    server.send(200, "text/plain", response);
}

void testBinaryPost() {
    Serial.println("Binary post detected...");

    Serial.println("Number of arguments...");
    Serial.println(server.args());
    Serial.println("Size of array...");
    Serial.println(sizeof(server.arg(0)));

    Serial.println("First byte read...");

    for(int x=0; x<sizeof(server.arg(0)); x++) {
      Serial.println(server.arg(0)[x], HEX);
    }

    Serial.println("Reading bytes...");
    //byte binArray[server.args(0).length];
    //server.args(0).getBytes(binArray, server.args(0).length);
    
    
  
    //Print out the amount of arguments received in POST payload
    Serial.println(server.args());
  
    server.send(200, "text/plain", "binary data sent");
    Serial.println("Binary post completed...");
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
