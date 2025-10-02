#include <Weather.h>
#include <serialCom.h>

bool weatherOK=false;

#ifdef WITH_AHT20
float intWeatherTemp = 0;
float intWeatherHum = 0;
bool AHTok = false;
#endif

const char* rootCACertificate = \
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"-----END CERTIFICATE-----\n";

//-------------------------------------------------------------------------------------
void weatherSetup() {
#ifdef WITH_AHT20
  Wire.setPins(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.begin();
  Wire.beginTransmission(AHT20_ADDRESS);
  Wire.write(0xBE);
  Wire.endTransmission();    // stop transmitting
  delay(40);
#endif
}

//-------------------------------------------------------------------------------------
bool ATH20_startSensor() {
  Wire.beginTransmission(AHT20_ADDRESS); // transmit to device #8
  Wire.write(0xac);
  Wire.write(0x33);
  Wire.write(0x00);
  Wire.endTransmission();    // stop transmitting
  unsigned long timer_s = millis();
  while(1) {
    if(millis()-timer_s > 200) return false;        // time out
    Wire.requestFrom(AHT20_ADDRESS, 1);
		while(Wire.available()) {
      unsigned char c = Wire.read();
      if(c&0x80 != 0)return true;      // busy
    }
    delay(20);
  }
}
 
//-------------------------------------------------------------------------------------
bool ATH20_getSensor_B(byte *h, int *t) {
  ATH20_startSensor();
  Wire.requestFrom(AHT20_ADDRESS, 6);
  unsigned char str[6];
  int index = 0;
  while (Wire.available()) {
      str[index++] = Wire.read(); // receive a byte as character
  }
  if(str[0] & 0x80) return false;
  int val;
  // According to humidity sensor accurate, one byte for humidity is OK
  val = 100 * str[1] / 255;
  *h = val; 
  // According to temperature sensor accurate, 12 bits is OK
  val = str[3]&0x0f;
  val <<=8;
  val += str[4];
	// temperature      = __temp / 4096.0 * 200.0 - 50.0;
	//                  = __temp / 20.48 - 50.0;
	// temperature * 10 = 10 * __temp / 20.48 - 500; // According to sensor accurate, we return temp*10 as int (0.1°C step)
	//long __temp = (long)val * 125 / 256 - 500;
	//*t = (int) __temp;
	*t = (long)val * 125 / 256 - 500;
  return true;
}

//-----------------------------------------------------------------------------
void getInternalWeather() {
//Serial.print("getInternalWeather\n");
//orderSend("SHOWVALUE","0|100|getIntWeather",false);
  byte humi;
  int temp;
//  int temp10;
//  int temp01;
//  char Buff[50];
  if(ATH20_getSensor_B(&humi, &temp)) {
    intWeatherHum = (float)humi;
    intWeatherTemp = (float)(temp/10.0);
//Serial.printf("indoor:: %4.1f'C %3.0f%%HR \n",intWeatherTemp,intWeatherHum);
  }
  else {
    intWeatherTemp = 0.0;
    intWeatherHum = 0.0;
  }
//char buf[40];
//sprintf(buf,"0|100|Fin/giw:%3.1f , %2.0f",intWeatherTemp,intWeatherHum);
//orderSend("SHOWVALUE",buf,false);
}

//-----------------------------------------------------------------------------
void getWeather() {
Serial.printf("getWeather wifiOK:%d weather_GPSlat=%f timezone[0]=%c\n",WiFiIsOK(),weather_GPSlat,weather_timezone[0]);
//orderSend("SHOWVALUE","0|60|getWeather",false);

//  if (internetOK && WiFiIsOK() && (weather_GPSlat != 0 || weather_GPSlon != 0) && weather_timezone[0]!='?') {
  if (WiFiIsOK() && (weather_GPSlat != 0 || weather_GPSlon != 0) && weather_timezone[0]!='?') {
    String serverPath = "https://api.open-meteo.com/v1/forecast?latitude=";
    serverPath += weather_GPSlat;
    serverPath += "&longitude=";
    serverPath += weather_GPSlon;
     serverPath += "&timezone=";
    serverPath += weather_timezone;
    serverPath += "&current=temperature_2m,relative_humidity_2m,precipitation,wind_speed_10m,wind_direction_10m";
    serverPath += "&forecast_days=3";
//    serverPath += "&hourly=temperature_2m,precipitation";
//    serverPath += "&daily=temperature_2m_max,temperature_2m_min,sunrise,sunset,uv_index_max,precipitation_sum";
    serverPath += "&daily=temperature_2m_max,temperature_2m_min,uv_index_max,precipitation_sum,wind_speed_10m_max,wind_direction_10m_dominant";
//Serial.print(serverPath);
//Serial.print(httpGETRequest(serverPath.c_str()));
    deserializeJson(weatherInfos, httpGETRequest(serverPath.c_str()));
    weatherOK = !(bool)weatherInfos["isKO"];
Serial.printf("getWeather weatherOK=%d \n",weatherOK);
  }
//Serial.printf("FIN/getWeather weatherOK=%d \n",weatherOK);
//orderSend("SHOWVALUE","0|60|Fin/getWeather",false);
}

//------ Interrogation HTTPS -------
String httpGETRequest(const char* serverName) {
//Serial.println(serverName);
  String payload = "{""isKO"":true}"; 
  WiFiClientSecure *client = new WiFiClientSecure;
  if (client) {
    client->setCACert(rootCACertificate);
    HTTPClient https;
    if (https.begin(*client, serverName)) {
      int httpResponseCode = https.GET();
      if (httpResponseCode == HTTP_CODE_OK || httpResponseCode == HTTP_CODE_MOVED_PERMANENTLY) {
//        Serial.print("HTTP Response code: ");
//        Serial.println(httpResponseCode);
        payload = https.getString();
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      https.end();
    }
  }
  return payload;
}

//-----------------------------------------------------------
//bool isWeatherOK() {
//  return weatherOK;
//}

//
