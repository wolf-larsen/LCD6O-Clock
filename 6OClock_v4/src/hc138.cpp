#include <Arduino.h>
#include <hc138.h>

void hc138Setup() {
  pinMode(HC138_A0_PIN, OUTPUT);
  digitalWrite(HC138_A0_PIN,LOW);
  pinMode(HC138_A1_PIN, OUTPUT);
  digitalWrite(HC138_A1_PIN,LOW);
  pinMode(HC138_A2_PIN, OUTPUT);
  digitalWrite(HC138_A2_PIN,LOW);
}

void hc138SetAdress(int pAdress) {
  digitalWrite(HC138_A0_PIN,(pAdress & 0B00000001) ? HIGH : LOW);
  digitalWrite(HC138_A1_PIN,(pAdress & 0B00000010) ? HIGH : LOW);
  digitalWrite(HC138_A2_PIN,(pAdress & 0B00000100) ? HIGH : LOW);
//if (!nodebug) Serial.printf("(%ld) HC138::activation ligne %d\n",millis(),pAdress);
}

