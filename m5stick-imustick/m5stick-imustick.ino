#include <WiFi.h>
#include <ArduinoOSCWiFi.h>

#include "FS.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

// - TODO: compile target switch
#define _TARGET_M5_STICKC_PLUS2_
// #define _TARGET_M5_STICKC_PLUS_

// - analogRead enable / disable switch
// #define _USE_ANALOG_READ_

#if defined(_TARGET_M5_STICKC_PLUS2_)
#include <M5StickCPlus2.h>
#define M5S StickCP2
#include <MadgwickAHRS.h>
Madgwick MadgwickFilter;
#define MagdwickHz 200
#endif

#if defined(_TARGET_M5_STICKC_PLUS_)
#include <M5StickCPlus.h>
#define M5S M5
#endif

const String config_filename = "/config.json";

String ssid = "ssid";
String pw = "password";
String osc_dest = "192.168.1.1";

float acc_threshold = 2.0;
float gyro_threshold = 2.0;
float ahrs_threshold = 1.0;
float g_threshold = 0.5;

// - normal mode OR config mode
int appMode = 1;

// - IMU
float accX = 0.0F;
float accY = 0.0F;
float accZ = 0.0F;
float g = 0.0F;

float gyroX = 0.0F;
float gyroY = 0.0F;
float gyroZ = 0.0F;

float magX = 0.0F;
float magY = 0.0F;
float magZ = 0.0F;

float pitch = 0.0F;
float roll  = 0.0F;
float yaw   = 0.0F;

// - Analog / Pressure
int v1 = 0;
int touch_elapsed = 0;

// - LCD ON/OFF state
int tgl = 0;

const float Vref = 3.3;
const int ADC_PIN = GPIO_NUM_36;

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

  const float _acc_threshold = doc["acc_t"];
  const float _gyro_threshold = doc["gyro_t"];
  const float _ahrs_threshold = doc["ahrs_t"];
  const float _g_threshold = doc["g_t"];
  acc_threshold = _acc_threshold;
  gyro_threshold = _gyro_threshold;
  ahrs_threshold = _ahrs_threshold;
  g_threshold = _g_threshold;

  return true;
}

bool saveConfig() {
  StaticJsonDocument<1024> doc;

  doc["ssid"] = ssid;
  doc["pw"] = pw;
  doc["osc_dest"] = osc_dest;

  doc["acc_t"] = acc_threshold;
  doc["gyro_t"] = gyro_threshold;
  doc["ahrs_t"] = ahrs_threshold;
  doc["g_t"] = g_threshold;

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

#if defined(_USE_ANALOG_READ_)
  pinMode(ADC_PIN, INPUT);
#endif

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

#if defined(_TARGET_M5_STICKC_PLUS_)
    M5S.Axp.begin();
    M5S.Axp.SetPeripherialsPower(0);
    M5S.Imu.Init();
#else
    M5S.Power.begin();
    MadgwickFilter.begin(MagdwickHz);
    M5S.Imu.init();
#endif

    // TODO:
    // OscWiFi.publish(osc_dest, 12000, "/stick/accel", accX, accY, accZ)->setFrameRate(24.f);
    // OscWiFi.publish(osc_dest, 12000, "/stick/gyro", gyroX, gyroY, gyroZ)->setFrameRate(24.f);
    // OscWiFi.publish(osc_dest, 12000, "/stick/ahrs", pitch, roll, yaw)->setFrameRate(24.f);
    // OscWiFi.publish(osc_dest, 12000, "/stick/shock", g)->setFrameRate(24.f);
    // OscWiFi.publish(osc_dest, 12000, "/stick/analog", v1, touch_elapsed)->setFrameRate(24.f);

    OscWiFi.subscribe(12000, "/acc_threshold", [&](float& th) {
      acc_threshold = th;

      if (saveConfig()) {
        OscWiFi.send(osc_dest, 12000, "/reply", "UPDATE: acc_threshold", th);
      }
    });

    OscWiFi.subscribe(12000, "/gyro_threshold", [&](float& th) {
      gyro_threshold = th;

      if (saveConfig()) {
        OscWiFi.send(osc_dest, 12000, "/reply", "UPDATE: gyro_threshold", th);
      }
    });

    OscWiFi.subscribe(12000, "/ahrs_threshold", [&](float& th) {
      ahrs_threshold = th;

      if (saveConfig()) {
        OscWiFi.send(osc_dest, 12000, "/reply", "UPDATE: ahrs_threshold", th);
      }
    });

    OscWiFi.subscribe(12000, "/g_threshold", [&](float& th) {
      g_threshold = th;

      if (saveConfig()) {
        OscWiFi.send(osc_dest, 12000, "/reply", "UPDATE: g_threshold", th);
      }
    });

    OscWiFi.subscribe(12000, "/current", []() {
      OscWiFi.send(
                   osc_dest, 12000,
                   "/reply",
                   "CURENT: acc_threshold, gyro_threshold, ahrs_threshold, g_threshold",
                   acc_threshold, gyro_threshold, ahrs_threshold, g_threshold
                   );
    });

    OscWiFi.subscribe(12000, "/preset", []() {
      acc_threshold = 2.0;
      gyro_threshold = 2.0;
      ahrs_threshold = 1.0;
      g_threshold = 0.5;

      if (saveConfig()) {
        OscWiFi.send(
                     osc_dest, 12000,
                     "/reply",
                     "PRESET: acc_threshold, gyro_threshold, ahrs_threshold, g_threshold",
                     acc_threshold, gyro_threshold, ahrs_threshold, g_threshold
                     );
      }
    });
  }
}


void loop() {
  float p_accX = accX;
  float p_accY = accY;
  float p_accZ = accZ;
  float p_g = g;

  float p_gyroX = gyroX;
  float p_gyroY = gyroY;
  float p_gyroZ = gyroZ;

  float p_pitch = pitch;
  float p_roll  = roll;
  float p_yaw   = yaw;

  int p_v1 = v1;
  int p_touch_elapsed = touch_elapsed;

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

#if defined(_TARGET_M5_STICKC_PLUS_)
    M5S.Imu.getAhrsData(&pitch, &roll, &yaw);
#else
    M5S.Imu.getMag(&magX, &magY, &magZ);
    MadgwickFilter.update(gyroX, gyroY, gyroZ, accX, accY, accZ, magX, magY, magZ);
    pitch = MadgwickFilter.getPitch();
    roll = MadgwickFilter.getRoll();
    yaw = MadgwickFilter.getYaw();
#endif

#if defined(_USE_ANALOG_READ_)
    v1 = analogRead(ADC_PIN);
    if (v1 > 3300) {
      touch_elapsed = min(touch_elapsed + 1,  100000000);
    } else {
      touch_elapsed = 0;
    }
#endif

    g = sqrt(pow(accX, 2) + pow(accY, 2) + pow(accZ, 2));

    if (abs(g - p_g) > g_threshold) {
      OscWiFi.send(osc_dest, 12000, "/shock", g, p_g, accX, p_accX, accY, p_accY, accZ, p_accZ);
    }

    if (abs(accX - p_accX) > acc_threshold || abs(accY - p_accY) > acc_threshold || abs(accZ - p_accZ) > acc_threshold) {
      OscWiFi.send(osc_dest, 12000, "/accel", accX, accY, accZ);
    }

    if (abs(gyroX - p_gyroX) > gyro_threshold || abs(gyroY - p_gyroY) > gyro_threshold || abs(gyroZ - p_gyroZ) > gyro_threshold) {
      OscWiFi.send(osc_dest, 12000, "/gyro", gyroX, gyroY, gyroZ);
    }

    if (abs(pitch - p_pitch) > ahrs_threshold || abs(roll - p_roll) > ahrs_threshold || abs(yaw - p_yaw) > ahrs_threshold) {
      OscWiFi.send(osc_dest, 12000, "/ahrs", pitch, roll, yaw);
    }

#if defined(_USE_ANALOG_READ_)
    if (abs(v1 - p_v1) > 2.0 || (touch_elapsed != p_touch_elapsed) ) {
      OscWiFi.send(osc_dest, 12000, "/analog", v1, touch_elapsed);
    }
#endif

    OscWiFi.update();

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
      M5S.Lcd.print("G(vec)");
    }

    delay(33);
  }
}
