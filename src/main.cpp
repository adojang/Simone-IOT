#include "Arduino.h"

#include <cmath>
#include <algorithm>
#include <iostream>
#include <vector>
/* 
 *  Customized Web LED Trigger Adapted from various sources.
 *  Adriaan van Wijk
 *  Anna-Mart Louw
 *  2022
 *  Web Integration Thanks to  https://www.hackster.io/donowak/internet-controlled-led-strip-using-esp32-arduino-2ca8a9
 *  Structure thanks to a final year thesis.
 * 
 * 
 * 
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <string>

void wifi_init();
void triggerGate(uint16_t delaytime);
void handle_OnConnect();
void handle_sendTime();
void handle_toggleLED();
void handle_exitconfig();
void handle_NotFound();
void handle_restartESP();
String SendHTML(uint8_t active);
String refreshpageHTML();
String restartpageHTML();
String ptr;

/* Put your SSID & Password */
const char *ssid_client = "Graves Into Gardens";    // Enter SSID here
const char *password_client = "throughchristalone"; //Enter Password here
const char *ssid_host = "Picotaal";                 // Enter SSID here
const char *password_host = "birdsfordays";         //Enter Password here

/* Put IP Address details */
IPAddress local_ip(10, 0, 0, 5);
IPAddress gateway(10, 10, 10, 1);
IPAddress subnet(255, 255, 255, 0);
WebServer server(80);

uint8_t LED_BUILTIN = 2; // Built in LED lights up when ESP is connected to Wifi.
uint8_t LED_TRIGGER = 23;
uint32_t TRIGGERTIME = 1500; // LED trigger time, in milliseconds.
/* ------------------------------------------- End of Constant Initialization --------------------------------------------- */

void setup()
{
  //EEPROM.begin(24);
  Serial.begin(115200); //Enable UART on ESP32

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_TRIGGER, OUTPUT);
  wifi_init(); //initialize wifi, runs once.
               /* A note about EEPROM 

        When starting with a FRESH ESP, the following MUST be implemented and flashed at least once, so that there is actual data there.
    */

  //EEPROM.put(0, float(7));
  //EEPROM.put(8, float(10));
  //EEPROM.commit();
  //EEPROM.put(18, 5000);
  //EEPROM.commit();
}

void loop()
{
  server.handleClient(); // handle the wifi page requests

  /*
                EEPROM.put(0, DIST_CUSTOM_IN);
                EEPROM.put(8, DIST_CUSTOM_OUT);
                EEPROM.commit();
            
                Serial.println("State saved in flash memory:");
                */

} // end of main loop

/* Functions Library: */

void wifi_station()
{
  WiFi.softAP(ssid_host, password_host);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  Serial.println("Wifi Hosted..!");
  digitalWrite(LED_BUILTIN, HIGH);
}

void wifi_client()
{
  /* Enable Station Mode Where it connects to a local Wifi */

  WiFi.begin(ssid_client, password_client);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());
}

void wifi_init() /* Wifi Intialization - Runs Once. */
{
  //Choose between self hosting a network, or connecting to an existing one.

  //wifi_station(); //Be the Router
  wifi_client(); //Connect to the Router

  /* WebServer Command Structure */
  server.on("/", handle_OnConnect);
  server.on("/toggle", handle_toggleLED);
  server.on("/exitconfig", handle_exitconfig);
  server.on("/sendTime", HTTP_POST, handle_sendTime);
  server.on("/restartESP", handle_restartESP);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");
}

/* Web Handling Functions */

void handle_OnConnect()
{
  server.send(200, "text/html", SendHTML(LOW)); //Not Active
}

void handle_sendTime()
{

  // This part just checks that the boxes are not blank. If they are blank when you click enter, it ignores the input.
  if (server.arg("custom_triggertime") != "")
    TRIGGERTIME = (server.arg("custom_triggertime").toInt());

  /*
    EEPROM.put(0, DIST_CUSTOM_IN);
    EEPROM.put(8, DIST_CUSTOM_OUT);
    EEPROM.put(18, time_delay);
    EEPROM.commit();
    delay(20); // Just in case
    Serial.println("State saved in flash memory:");
    */
  Serial.println("LED Triggered!");
  server.send(200, "text/html", refreshpageHTML());
}

void handle_toggleLED()
{
  //This toggle
  digitalWrite(LED_TRIGGER, HIGH);
  server.send(200, "text/html", SendHTML(HIGH)); //Active
  delay(1500);
  Serial.printf("The value for value is %d\n", TRIGGERTIME);
  digitalWrite(LED_TRIGGER, LOW);
  handle_OnConnect();
}

void handle_exitconfig()
{
  //Put a cute disconnect loading page here that delays a bit before disconnecting
  handle_OnConnect();
  Serial.println("\nDisconnecting from AP now");
  //WiFi.softAPdisconnect(true);
}

void handle_restartESP()
{
  server.send(200, "text/html", restartpageHTML());
  WiFi.softAPdisconnect(true);
  delay(100);
  ESP.restart();
}

void handle_NotFound()
{
  server.send(404, "text/plain", "Not found. Are you sure you're surfing the right web?");
}

/* Web Page Design */

String SendHTML(uint8_t active)
{

  ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>Gate Control</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr += ".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".buttonsmall {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 18px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";

  ptr += ".button-on {background-color: #d74242;}\n";
  ptr += ".button-on:active {background-color: #d74242;}\n";
  ptr += ".button-off {background-color: #852b2b;}\n";
  ptr += ".button-off:active {background-color: #2c3e50;}\n";
  ptr += ".button-save {background-color: #429bd7;}\n";
  ptr += ".button-restart {background-color: #ff3d00;}\n";

  ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "</style>\n";

  if (active)
  {
    ptr += "<meta http-equiv='refresh' content='2;url=/'>\n";
  }

  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>SimoneIOT Light Control</h1>\n";

  /* Toggle Gate*/
  if (active)
  {
    ptr += "<p>Pushing Button</p><a class=\"button button-off\" href=\"/\">Gate Opening</a>\n";
  }
  else
  {
    ptr += "<p>Button Released</p><a class=\"button button-on\" href=\"/toggle\">Toggle Light</a>\n";
  }

  ptr += "<p>Time Delay: " + String(TRIGGERTIME) + "[ms] </p>\n";

  /* Form and Button Part*/
  ptr += "<form action=\"/sendTime\" method=\"POST\">";
  ptr += "<input type = \"number\" name = \"custom_triggertime\" placeholder = \"Trigger Time [ms]\"><br>";
  ptr += "<br><input type = \"submit\" value = \"Enter\">";
  ptr += "</form>";

  /*Save and Quit*/
  //This doesn't actually do anything...
  //ptr += "<a class=\"buttonsmall button-save\" href=\"/exitconfig\">Refresh Page</a>\n";

  /*Footers*/
  ptr += "<h6>Created by A. van Wijk and A. Louw 2022</h6>\n";

  ptr += "<a class=\"buttonsmall button-restart\" href=\"/restartESP\">[Restart ESP]</a>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}

String refreshpageHTML()
{

  ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>Gate Control</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center; vertical-align: middle;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr += ".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".buttonsmall {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 18px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".button-on {background-color: #d74242;}\n";
  ptr += ".button-on:active {background-color: #d74242;}\n";
  ptr += ".button-off {background-color: #852b2b;}\n";
  ptr += ".button-off:active {background-color: #2c3e50;}\n";
  ptr += ".button-save {background-color: #429bd7;}\n";
  ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "</style>\n";

  ptr += "<meta http-equiv='refresh' content='2;url=/'>\n";

  ptr += "</head>\n";
  ptr += "<body>\n";

  ptr += "<h1>Value Saved!</h1>\n";

  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}

String restartpageHTML()
{

  ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>Gate Control</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center; vertical-align: middle;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr += ".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".buttonsmall {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 18px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".button-on {background-color: #d74242;}\n";
  ptr += ".button-on:active {background-color: #d74242;}\n";
  ptr += ".button-off {background-color: #852b2b;}\n";
  ptr += ".button-off:active {background-color: #2c3e50;}\n";
  ptr += ".button-save {background-color: #429bd7;}\n";
  ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "</style>\n";

  ptr += "<meta http-equiv='refresh' content='15;url=/'>\n";

  ptr += "</head>\n";
  ptr += "<body>\n";

  ptr += "<h1>Restarting ESP</h1>\n";
  ptr += "<h2>Please wait for 15 seconds...</h2>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}