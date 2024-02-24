#include <WiFi.h>
#include <ArduinoOSCWiFi.h>

#include "FS.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

const String config_filename = "/config.json";

String ssid = "ssid";
String pw = "password";
String osc_dest = "192.168.1.1";

int appMode = 1;

int v1 = 0;
int touch_elapsed = 0;

int tgl = 0;

#define PIN_BUTTON 0
#define ADC_PIN GPIO_NUM_3

const float Vref = 3.3;
// const int ADC_PIN = GPIO_NUM_3;

bool readConfig() {
  String file_content = readFile(config_filename);

  int config_file_size = file_content.length();
  Serial.println("Config file size: " + String(config_file_size));

  if(config_file_size > 1024) {
    Serial.println("Config file too large");
    return false;
  }
  StaticJsonDocument<1024> doc;
  auto error = deserializeJson(doc, file_content);
  if ( error ) {
    Serial.println("Error interpreting config file");
    return false;
  }
  const String _ssid = doc["ssid"];
  const String _pw = doc["pw"];
  const String _osc_dest = doc["osc_dest"];
  ssid = _ssid;
  pw = _pw;
  osc_dest = _osc_dest;

  return true;
}

bool saveConfig() {
  StaticJsonDocument<1024> doc;

  doc["ssid"] = ssid;
  doc["pw"] = pw;
  doc["osc_dest"] = osc_dest;

  String tmp = "";
  serializeJson(doc, tmp);
  writeFile(config_filename, tmp);

  return true;
}

void writeFile(String filename, String message){
  File file = LittleFS.open(filename, "w");
  if(!file){
    Serial.println("writeFile -> failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

String readFile(String filename){
  File file = LittleFS.open(filename);
  if(!file){
    Serial.println("Failed to open file for reading");
    return "";
  }

  String fileText = "";
  while(file.available()){
    fileText = file.readString();
  }
  file.close();
  return fileText;
}

void setup() {
  Serial.begin(115200);

  pinMode(ADC_PIN, INPUT);

  if (!LittleFS.begin(false)) {
    Serial.println("LITTLEFS Mount failed");
    Serial.println("Did not find filesystem; starting format");

    if (!LittleFS.begin(true)) {
      Serial.println("LITTLEFS mount failed");
      Serial.println("Formatting not possible");
      return;
    }
  } else {
    Serial.println("setup -> SPIFFS mounted successfully");
    if(readConfig() == false) {
      Serial.println("setup -> Could not read Config file -> initializing new file");
      if (saveConfig()) {
        Serial.println("setup -> Config file saved");
      }
    }

    Serial.println("ssid = " + ssid + ", pw = " + pw + ", osc_dest = " + osc_dest);
  }
}


void loop() {
  int p_v1 = v1;
  int p_touch_elapsed = touch_elapsed;

  if (!digitalRead(PIN_BUTTON)) {
    delay(100);
    if (!digitalRead(PIN_BUTTON)) {
      appMode = (appMode + 1) % 2;

      if (appMode == 0) {
        Serial.println("Enter configuration mode");
      } else {
        Serial.println("Exit configuration mode");
      }
    }
  }

  if (appMode == 0) {
    if (Serial.available() > 0) {
      String tmp_str = Serial.readString();
      String tmp_perms[3] = { "", "", "" };
      int sidx = 0, pidx = 0;

      while (true) {
        int fidx = tmp_str.indexOf(",", sidx);
        if (fidx != -1) {
          String sstr = tmp_str.substring(sidx, fidx);
          sidx = fidx + 1;
          if (pidx < 3) {
            tmp_perms[pidx] = sstr;
            pidx ++;
          } else {
            break;
          }
        } else {
          String rstr = tmp_str.substring(sidx, tmp_str.length() - 1);
          if (pidx < 3) {
            tmp_perms[pidx] = rstr;
          }
          break;
        }
      }

      if (tmp_perms[0].length() != 0) {
        ssid = tmp_perms[0];
        Serial.println("ssid update -> " + ssid);
      }

      if (tmp_perms[1].length() != 0) {
        pw = tmp_perms[1];
        Serial.println("pw update -> " + pw);
      }

      if (tmp_perms[2].length() != 0) {
        osc_dest = tmp_perms[2];
        Serial.println("osc_dst update -> " + osc_dest);
      }

      if (saveConfig()) {
        Serial.println("setup -> Config file saved");
      }

      Serial.println(tmp_str);
    }
  } else {
    v1 = analogRead(ADC_PIN);

    if (v1 > 3300) {
      touch_elapsed = min(touch_elapsed + 1,  100000000);
    } else {
      touch_elapsed = 0;
    }

    if (abs(v1 - p_v1) > 2.0 || (touch_elapsed != p_touch_elapsed) ) {
      OscWiFi.send(osc_dest, 12000, "/analog", v1, touch_elapsed);
    }

    OscWiFi.update();
  }
}
