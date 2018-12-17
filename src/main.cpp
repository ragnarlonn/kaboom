#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "kaboom.h"

// Configure your SSID name and password here
const char *ssid = "Kaboom AP";
const char *pw = "secret314";

// Configure your firing channels here. Each NodeMCU pin you configure will be set HIGH for 2 seconds
// after its firing button is clicked in the web UI.
//
// Each firing channel should be added as a new line in the array below. Copy/change an existing
// entry and only change the pin number (first field) and name (second field).
//
// Information on pin numbers: 
// Use the NodeMCU GPIO pin number as the pin number here. I.e. Digital pin D0 == 16 (GPIO16)
// (Google "NodeCU pinout" to get a reference drawing)
struct DetonatorChannel channels[] = {
  { 16, "Falcon 9.5", false, false, false, millis() },
  { 5, "My rocket on pin D1", false, false, false, millis() }
};

// No more configuration necessary from here on

//WiFiServer server(80);
ESP8266WebServer server(80);
String httpRequest;
String htmlout;
const unsigned int numChannels = (sizeof(channels) / sizeof(struct DetonatorChannel));

void handleRoot() {
  String s = html1;
  char tmp[300];
  for (unsigned int i=0; i<numChannels; i++) {
    bool fired = channels[i].fired;
    bool error = channels[i].error;
    snprintf(tmp, 300, "<tr><td>%s</td><td>%d</td>\
      <td id=\"firing%d\" style=\"background-color:#white; color:#black\"></td>\
      <td id=\"fired%d\" style=\"background-color: %s; color:%s;\">%s</td>\
      <td><button id=\"button%d\" onclick=\"fire(event)\" %s>fire</button></td></tr>",
      channels[i].name.c_str(), channels[i].pinNumber, 
      channels[i].pinNumber,
      channels[i].pinNumber, fired ? (error ? "#red" : "#green") : "#white", "#black", 
      fired ? (error ? "<b><i>ERR</i></b>" : "<b>OK</b>") : "",
      channels[i].pinNumber, fired ? "disabled" : "");
    s += tmp;
  }
  s += html2;
  server.send(200, "text/html", s);
}

void handleFire() {
  int pin = -1;
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "pin") {
      pin = atoi(server.arg(i).c_str());
      break;
    }
  }
  if (pin == -1) {
    server.send(400, "text/plain", "Seriously?");
    Serial.println("Got FIRE command without pin parameter, ignoring");
    return;
  }
  for (uint8_t i = 0; i < numChannels; i++) {
    if (channels[i].pinNumber == pin) {
      Serial.print("Got FIRE command for pin "); Serial.print(pin); Serial.println(" - EXECUTING");
      channels[i].changed = millis();
      channels[i].on = true;
      digitalWrite(pin, HIGH);
      server.send(200, "text/plain", "KABOOM");
      return;
    }
  }
  server.send(404, "text/plain", "KEEP TRACK OF YOUR FIREWORKS!");
  Serial.print("Got FIRE command for non-existent pin number "); Serial.print(pin); Serial.println(", ignoring");
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  for (unsigned int i = 0; i < 6; i++) {
    Serial.println(title[i]);
  }
  Serial.println();
  Serial.println("Hello, this is your friendly nodemcu detonation controller");
  Serial.println("Initializing outputs...");
  for (unsigned int i = 0; i<numChannels; i++) {
    Serial.print("Initializing output pin #"); Serial.print(channels[i].pinNumber); 
    Serial.print(" ("); Serial.print(channels[i].name); Serial.print(") ...");
    pinMode(channels[i].pinNumber, OUTPUT);
    digitalWrite(channels[i].pinNumber, LOW);
    channels[i].on = channels[i].fired = false;
    Serial.println("ok");
  }
  Serial.println("Init outputs done");
  Serial.print("Setting up AP...");
  boolean res = WiFi.softAP(ssid, pw);
  if (res == true) {
    Serial.print("ok, IP="); Serial.print(WiFi.softAPIP()); 
    Serial.print(", SSID="); Serial.print(WiFi.softAPSSID());
    Serial.print(", pw="); Serial.println(pw);
    server.on("/fire", handleFire);
    server.on("/", handleRoot);
    server.begin();
    Serial.print("Ready for action. Fire control available at http://"); Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("FAILED!");
  }
}


void release() {
  for (unsigned int i = 0; i < numChannels; i++) {
    if (channels[i].on) {
      if (millis() > (channels[i].changed + 2000)) {
        // switch off relay after 2 secs
        Serial.println("");
        Serial.print("Setting output pin "); Serial.print(channels[i].pinNumber); Serial.println(" to LOW");
        digitalWrite(channels[i].pinNumber, LOW);
        channels[i].on = false;
        channels[i].fired = true;
        channels[i].changed = millis();
      }
    }
  }
}

void loop() {
  server.handleClient();
  release();
}