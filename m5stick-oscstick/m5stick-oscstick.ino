#include <WiFi.h>
#include <ArduinoOSCWiFi.h>

#include "FS.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

// - TODO:
// #define _TARGET_M5_STICKC_PLUS2_
#define _TARGET_M5_STICKC_PLUS_

#if defined(_TARGET_M5_STICKC_PLUS2_)
#include <M5StickCPlus2.h>
#define M5S StickCP2
#endif

#if defined(_TARGET_M5_STICKC_PLUS_)
#include <M5StickCPlus.h>
#define M5S M5
#endif

const String config_filename = "/config.json";

String ssid = "ssid";
String pw = "password";
String osc_dest = "192.168.1.1";

int appMode = 1;

float accX = 0.0F;
float accY = 0.0F;
float accZ = 0.0F;
float g = 0.0F;

float gyroX = 0.0F;
float gyroY = 0.0F;
float gyroZ = 0.0F;

float pitch = 0.0F;
float roll  = 0.0F;
float yaw   = 0.0F;

int tgl = 0;

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
  M5S.begin();

  M5S.Lcd.setRotation(3);
  M5S.Lcd.fillScreen(BLACK);
  M5S.Lcd.setTextSize(1);

  M5S.update();

  Serial.begin(115200);

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

  if (M5S.BtnA.isPressed()) {
    M5S.Lcd.setCursor(60, 15);
    M5S.Lcd.println("Enter configuration mode");

    appMode = 0;
  } else {
    M5S.Lcd.setCursor(30, 15);
    M5S.Lcd.println("OSC://" + osc_dest + ":12000");
    M5S.Lcd.setCursor(30, 35);
    M5S.Lcd.println("  X       Y       Z");
    M5S.Lcd.setCursor(30, 75);
    M5S.Lcd.println("  Pitch   Roll    Yaw");

    WiFi.begin(ssid, pw);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
    }

    OscWiFi.publish(osc_dest, 12000, "/acc/x", accX);
    OscWiFi.publish(osc_dest, 12000, "/acc/y", accY);
    OscWiFi.publish(osc_dest, 12000, "/acc/z", accZ);
    OscWiFi.publish(osc_dest, 12000, "/shock", g);

#if defined(_TARGET_M5_STICKC_PLUS_)
    M5S.Axp.begin();
    M5S.Axp.SetPeripherialsPower(0);
    M5S.Imu.Init();
#else
    M5S.Power.begin();
    M5S.Imu.init();
#endif
  }
}


void loop() {
  M5S.update();

  if (M5S.BtnA.wasPressed()) {
    tgl = (tgl + 1) % 2;

    if (tgl == 0) {
#if defined(_TARGET_M5_STICKC_PLUS_)
      M5S.Axp.ScreenSwitch(true);
#else
      M5S.Lcd.powerSaveOff();
      M5S.Lcd.setBrightness(255);
#endif
    } else {
#if defined(_TARGET_M5_STICKC_PLUS_)
      M5S.Axp.ScreenSwitch(false);
#else
      M5S.Lcd.powerSaveOn();
      M5S.Lcd.setBrightness(0);
#endif
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
    M5S.Imu.getGyroData(&gyroX, &gyroY, &gyroZ);
    M5S.Imu.getAccelData(&accX, &accY, &accZ);
    // - TODO:
    // M5S.Imu.getAhrsData(&pitch,&roll,&yaw);
    // M5S.Imu.getMag(&pitch,&roll,&yaw);

    g = sqrt(pow(accX, 2) + pow(accY, 2) + pow(accZ, 2)) * 1000;

    if (tgl == 0) {
      M5S.Lcd.setCursor(30, 45);
      M5S.Lcd.printf("%6.2f  %6.2f  %6.2f      ", gyroX, gyroY, gyroZ);
      M5S.Lcd.setCursor(170, 45);
      M5S.Lcd.print("o/s");
      M5S.Lcd.setCursor(30, 55);
      M5S.Lcd.printf(" %5.2f   %5.2f   %5.2f   ", accX, accY, accZ);
      M5S.Lcd.setCursor(170, 55);
      M5S.Lcd.print("G");
      M5S.Lcd.setCursor(30, 85);
      M5S.Lcd.printf(" %5.2f   %5.2f   %5.2f   ", pitch, roll, yaw);
      M5S.Lcd.setCursor(30, 105);
      M5S.Lcd.printf("  %.1f", g);
      M5S.Lcd.setCursor(100, 105);
      M5S.Lcd.print("mG");
    }

    OscWiFi.update();
    delay(33);
  }
}
