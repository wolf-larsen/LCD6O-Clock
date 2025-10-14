#include "defines.h"
#include "WiFiSecondarySetup.h"

IPAddress ip;

bool WiFiSetup() {
  Serial.println("wifi setup");
  WiFi.setMinSecurity(WIFI_AUTH_WPA_PSK);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  WiFi.setMinSecurity(WIFI_AUTH_WPA_PSK);
  WiFi.begin(wifi_SSID, wifi_password);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  int8_t nbRetry=20;
  Serial.printf("Connecting to: %s  ",wifi_SSID);
  while (WiFi.status() != WL_CONNECTED && nbRetry-->=0) {
    delay(1000);
    Serial.print('.');
  }
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect();
    Serial.print("\nUnable to connect to:");
    Serial.println(wifi_SSID);
    return false;
  }
  Serial.println(" OK");
  ip = WiFi.localIP();
  Serial.printf("IP: %d.%d.%d.%d \n",ip[0],ip[1],ip[2],ip[3]);
  return true;
}

bool WiFiIsOK() {
  return (WiFi.status() == WL_CONNECTED);
}

//
