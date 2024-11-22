#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>
#include "GSMSIM800L.h"

#define ONE_WIRE_BUS 4

SoftwareSerial gsmSerial(2, 3); // RX, TX pins
GSMSIM800L sim800l(&gsmSerial);
String ContactNumber = "+639501057592";

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int PHsensor = A0;
int TDSsensor = A1;

LiquidCrystal_I2C lcd(0x27, 16, 2);

struct Thresholds {
  float low;
  float high;
};

Thresholds phThreshold = {5.0, 7.0};
Thresholds tdsThreshold = {80.0, 140.0};
Thresholds tempThreshold = {26, 31};

const int tempRelayA = 5;
const int tempRelayB = 6;
const int phRelayA = 7;
const int phRelayB = 8;
const int tdsRelayA = 9;
const int tdsRelayB = 10;

bool tempState = false; 
bool phState = false;
bool tdsState = false;

void setup(){
  Serial.begin(9600);
  sensors.begin();
  gsmSerial.begin(9600); 

  sim800l.init();
  sim800l.setDebugging(true);
  sim800l.SendSMS("Device has Started", ContactNumber);

  lcd.begin();
  lcd.backlight();
  lcd.print("LCD Initiate!");
  delay(2000);
  lcd.clear();

  pinMode(tempRelayA, OUTPUT);
  pinMode(tempRelayB, OUTPUT);
  pinMode(phRelayA, OUTPUT);
  pinMode(phRelayB, OUTPUT);
  pinMode(tdsRelayA, OUTPUT);
  pinMode(tdsRelayB, OUTPUT);

  resetRelays();
}

void loop(){
  float temp = readTemperature();
  float phLevel = readPH();
  float tds = readTDS();

  displayReadings(phLevel, temp, tds);
  
  controlRelays(temp, tempThreshold, tempRelayA, tempRelayB, "Temp", tempState);
  controlRelays(phLevel, phThreshold, phRelayA, phRelayB, "pH", phState);
  controlRelays(tds, tdsThreshold, tdsRelayA, tdsRelayB, "TDS", tdsState);

  delay(1000);
}

float readTemperature() {
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(0);
}

float readPH() {
  int PHraw = analogRead(PHsensor);
  float phLevel = fmap(PHraw, 380, 310, 7, 4);
  return constrain(phLevel, 0, 14);
}

float readTDS() {
  int TDSraw = analogRead(TDSsensor);
  int tds = map(TDSraw, 38, 27, 91, 68);
  return constrain(tds, 0, 1200);
}

void displayReadings(float ph, float temp, float tds) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("pH: " + String(ph, 1) + " T: " + String(temp, 1) + "C");
  
  lcd.setCursor(0, 1);
  lcd.print("TDS: " + String(tds, 1) + " ppm");

  Serial.println("pH Level: " + String(ph, 1));
  Serial.println("Temperature: " + String(temp, 1));
  Serial.println("TDS: " + String(tds, 1));
}

void controlRelays(float value, Thresholds thresholds, int relayA, int relayB, String label, bool &state) {
  bool newState = false;

  if (value < thresholds.low) {
    digitalWrite(relayA, LOW);
    digitalWrite(relayB, HIGH);
    newState = true;
    Serial.println(label + " Relay A High; " + label + " Relay B Low");
  } else if (value > thresholds.high) {
    digitalWrite(relayA, HIGH);
    digitalWrite(relayB, LOW);
    newState = true;
    Serial.println(label + " Relay A Low; " + label + " Relay B High");
  } else {
    digitalWrite(relayA, HIGH);
    digitalWrite(relayB, HIGH);
    Serial.println(label + " Relay A Low; " + label + " Relay B Low");
  }

  if (newState != state) {
    controlSMS(value, thresholds, newState, label);
    state = newState;
  }
}

void controlSMS(float value, Thresholds thresholds, bool state, String label) {
  String message = label + " Sensor is ";

  if (value < thresholds.low) {
    message += "BELOW threshold. Value: " + String(value);
  } else if (value > thresholds.high) {
    message += "ABOVE threshold. Value: " + String(value);
  } else {
    message += "within normal range. Value: " + String(value);
  }

  sim800l.SendSMS(message, ContactNumber);
  Serial.println("SMS Sent: " + message);
}

void resetRelays() {
  digitalWrite(tempRelayA, HIGH);
  digitalWrite(tempRelayB, HIGH);
  digitalWrite(phRelayA, HIGH);
  digitalWrite(phRelayB, HIGH);
  digitalWrite(tdsRelayA, HIGH);
  digitalWrite(tdsRelayB, HIGH);
}

float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int map(int x, int in_min, int in_max, int out_min, int out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
