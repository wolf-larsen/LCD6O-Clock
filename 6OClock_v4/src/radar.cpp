#include <Arduino.h>
#include <radar.h>

ld2410 radar;

bool radarConnected = false;


void radarSetup() {
  RADAR_SERIAL.begin(256000, SERIAL_8N1, RSERIAL_RX_PIN, RSERIAL_TX_PIN);
  delay(500);
  if (radar.begin(RADAR_SERIAL)) {
    radarConnected = true;
    Serial.println(F("Radar OK"));
    Serial.print(F("LD2410 firmware version: "));
    Serial.print(radar.firmware_major_version);
    Serial.print('.');
    Serial.print(radar.firmware_minor_version);
    Serial.print('.');
    Serial.println(radar.firmware_bugfix_version, HEX);
  }
  else {
    radarConnected = false;
    Serial.println(F("Radar not connected"));
  }
}

int radarDetection() {
  radar.read();
  if (radar.isConnected()) {
    if (radar.presenceDetected()) {
      if (radar.stationaryTargetDetected()) {
//        Serial.print(F("Stationary target: "));
//        Serial.print(radar.stationaryTargetDistance());
//        Serial.print(F("cm energy:"));
//        Serial.print(radar.stationaryTargetEnergy());
//        Serial.print(' ');
//        Serial.println();
        return 1;
      }
      else if (radar.movingTargetDetected()) {
//        Serial.print(F("Moving target: "));
//        Serial.print(radar.movingTargetDistance());
//        Serial.print(F("cm energy:"));
//        Serial.print(radar.movingTargetEnergy());
//        Serial.println();
        return 2;
      }
      else {
        return 20;
      }
    }
    else {
//      Serial.println(F("No target"));
      return 0;
    }
  }
  else {
//    Serial.println(F("No radar"));
    return 10;
  }
}

