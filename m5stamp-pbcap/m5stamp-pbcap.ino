#include <Wire.h>

#include <MadgwickAHRS.h>
Madgwick MadgwickFilter;
#include <Ticker.h>
Ticker magdwickticker;

#include "FS.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

#include <WiFi.h>
#include <ArduinoOSCWiFi.h>

#define MagdwickHz 100
const float magdwickinterval = 1000 / MagdwickHz;

// BMX055
#define Addr_Accl 0x19
#define Addr_Gyro 0x69
#define Addr_Mag 0x13

float ax = 0.00;
float ay = 0.00;
float az = 0.00;
float gx = 0.00;
float gy = 0.00;
float gz = 0.00;
int mx = 0.00;
int my = 0.00;
int mz = 0.00;
float pitch = 0.00;
float roll = 0.00;
float yaw = 0.00;

float g = 0.0F;

const String config_filename = "/config.json";

String ssid = "ssid";
String pw = "password";
String osc_dest = "192.168.1.1";

int appMode = 1;

#define PIN_BUTTON 0

void BMX055_Init()
{
  Wire.beginTransmission(Addr_Accl);
  Wire.write(0x0F); // Select PMU_Range register
  Wire.write(0x03);   // Range = +/- 2g
  Wire.endTransmission();
  delay(100);
  //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Accl);
  Wire.write(0x10);  // Select PMU_BW register
  Wire.write(0x08);  // Bandwidth = 7.81 Hz
  Wire.endTransmission();
  delay(100);
  //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Accl);
  Wire.write(0x11);  // Select PMU_LPW register
  Wire.write(0x00);  // Normal mode, Sleep duration = 0.5ms
  Wire.endTransmission();
  delay(100);
  //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Gyro);
  Wire.write(0x0F);  // Select Range register
  Wire.write(0x04);  // Full scale = +/- 125 degree/s
  Wire.endTransmission();
  delay(100);
  //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Gyro);
  Wire.write(0x10);  // Select Bandwidth register
  Wire.write(0x07);  // ODR = 100 Hz
  Wire.endTransmission();
  delay(100);
  //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Gyro);
  Wire.write(0x11);  // Select LPM1 register
  Wire.write(0x00);  // Normal mode, Sleep duration = 2ms
  Wire.endTransmission();
  delay(100);
  //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Mag);
  Wire.write(0x4B);  // Select Mag register
  Wire.write(0x83);  // Soft reset
  Wire.endTransmission();
  delay(100);
  //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Mag);
  Wire.write(0x4B);  // Select Mag register
  Wire.write(0x01);  // Soft reset
  Wire.endTransmission();
  delay(100);
  //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Mag);
  Wire.write(0x4C);  // Select Mag register
  Wire.write(0x00);  // Normal Mode, ODR = 10 Hz 0x00 //100Hz 0x07
  Wire.endTransmission();
  //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Mag);
  Wire.write(0x4E);  // Select Mag register
  Wire.write(0x84);  // X, Y, Z-is enabled
  Wire.endTransmission();
  //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Mag);
  Wire.write(0x51);  // Select Mag register
  Wire.write(0x04);  // No. of Repetitions for X-Y is = 9
  Wire.endTransmission();
  //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Mag);
  Wire.write(0x52);  // Select Mag register
  Wire.write(0x16);  // No. of Repetitions for Z-Axis = 15
  Wire.endTransmission();
}

void BMX055_All()
{
  unsigned int data[8];

  // Acceleraton
  for (int i = 0; i < 6; i++)
  {
    Wire.beginTransmission(Addr_Accl);
    Wire.write((2 + i));// Select data register
    Wire.endTransmission();
    Wire.requestFrom(Addr_Accl, 1);// Request 1 byte of data
    // Read 6 bytes of data
    // ax lsb, ax msb, ay lsb, ay msb, az lsb, az msb
    if (Wire.available() == 1)
      data[i] = Wire.read();
  }
  // Convert the data to 12-bits
  ax = ((data[1] * 256) + (data[0] & 0xF0)) / 16;
  if (ax > 2047)  ax -= 4096;
  ay = ((data[3] * 256) + (data[2] & 0xF0)) / 16;
  if (ay > 2047)  ay -= 4096;
  az = ((data[5] * 256) + (data[4] & 0xF0)) / 16;
  if (az > 2047)  az -= 4096;
  ax = ax * 0.00098; // range = +/-2g
  ay = ay * 0.00098; // range = +/-2g
  az = az * 0.00098; // range = +/-2g

  // Gyro
  for (int i = 0; i < 6; i++)
  {
    Wire.beginTransmission(Addr_Gyro);
    Wire.write((2 + i));    // Select data register
    Wire.endTransmission();
    Wire.requestFrom(Addr_Gyro, 1);    // Request 1 byte of data
    if (Wire.available() == 1)
      data[i] = Wire.read();
  }
  // Convert the data
  gx = (data[1] * 256) + data[0];
  if (gx > 32767)  gx -= 65536;
  gy = (data[3] * 256) + data[2];
  if (gy > 32767)  gy -= 65536;
  gz = (data[5] * 256) + data[4];
  if (gz > 32767)  gz -= 65536;

  gx = gx * 0.0038; //  Full scale = +/- 125 degree/s
  gy = gy * 0.0038; //  Full scale = +/- 125 degree/s
  gz = gz * 0.0038; //  Full scale = +/- 125 degree/s

  // Compass
  for (int i = 0; i < 8; i++)
  {
    Wire.beginTransmission(Addr_Mag);
    Wire.write((0x42 + i));    // Select data register
    Wire.endTransmission();
    Wire.requestFrom(Addr_Mag, 1);    // Request 1 byte of data
    // Read 6 bytes of data
    // mx lsb, mx msb, my lsb, my msb, mz lsb, mz msb
    if (Wire.available() == 1)
      data[i] = Wire.read();
  }
  // Convert the data
  mx = ((data[1] << 5) | (data[0] >> 3));
  if (mx > 4095)  mx -= 8192;
  my = ((data[3] << 5) | (data[2] >> 3));
  if (my > 4095)  my -= 8192;
  mz = ((data[5] << 7) | (data[4] >> 1));
  if (mz > 16383)  mz -= 32768;

  MadgwickFilter.update(gx, gy, gz, ax, ay, az, mx, my, mz);
  pitch = MadgwickFilter.getPitch();
  roll  = MadgwickFilter.getRoll();
  yaw   = MadgwickFilter.getYaw();
}

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

void setup()
{
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

  MadgwickFilter.begin(MagdwickHz);
  Wire.begin();
  BMX055_Init();
  magdwickticker.attach_ms(magdwickinterval, BMX055_All);
  delay(300);

  WiFi.begin(ssid, pw);

  int timeout = 0;

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Waiting connection..");
    timeout ++;
    if (timeout > 20) {
      break;
    }
  }

  OscWiFi.publish(osc_dest, 12000, "/acc/x", ax);
  OscWiFi.publish(osc_dest, 12000, "/acc/y", ay);
  OscWiFi.publish(osc_dest, 12000, "/acc/z", az);
  OscWiFi.publish(osc_dest, 12000, "/shock", g);

  Serial.println("Enter loop");
}

void loop()
{

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
    g = sqrt(pow(ax, 2) + pow(ay, 2) + pow(az, 2)) * 1000;

    OscWiFi.update();
    // delay(33);
    delay(100);
  }
}
