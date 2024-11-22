#include "GSMSIM800L.h"

GSMSIM800L::GSMSIM800L(Stream *serial) {
  gsmSerial = serial;
  gsmSerial->println("AT"); // Initial command to ensure serial is functional
}

void GSMSIM800L::init() {
  Serial.println("Initializing GSM SIM800L");
  
  while(ATResponse.indexOf("OK") < 0) {
    ATResponse = ATCommand("AT");
  }
  ATResponse = ATCommand("AT+CMGF=1");
  ATResponse = ATCommand("AT+CMGDA=\"DEL ALL\"");
  ATResponse = ATCommand("AT+CNMI=1,2,0,0,0");
}

String GSMSIM800L::monitorGSMSerial() {
  delay(500);
  String response = "";
  while (gsmSerial->available()) {
    response = gsmSerial->readString();
  }
  if (debug_mode && response != "") {
    Serial.println("-----------------------");
    Serial.println("RESPONSE: " + response);
    Serial.println("-----------------------");
  }

  if (response.indexOf("+CMT:") > -1) {
    int startIdx = response.indexOf("\"") + 1;
    int endIdx = response.indexOf("\"", startIdx);
    senderNumber = response.substring(startIdx, endIdx);

    if (debug_mode) {
      Serial.println("Sender Number: " + senderNumber);
    }

    byte delimiter = response.indexOf('\n', response.indexOf("+CMT:") + 1);
    SMSBuffer = response.substring(delimiter + 1);
    receivedSMS = true;
    
    if (debug_mode) {
      Serial.println("SMS RECEIVED: " + SMSBuffer);
    }
  }
  return response;
}

String GSMSIM800L::ATCommand(String cmd) {
  gsmSerial->println(cmd);
  return monitorGSMSerial();
}

String GSMSIM800L::getReceivedSMS() {
  receivedSMS = false;
  return SMSBuffer;
}

void GSMSIM800L::setDebugging(bool mode) {
  debug_mode = mode;
}

void GSMSIM800L::SendSMS(String msg, String num) {
  if (debug_mode)
    Serial.println("Sending SMS: " + msg + " to " + num);

  ATResponse = ATCommand("AT+CMGF=1\r");
  delay(100);

  ATResponse = ATCommand("AT+CMGS=\"" + String(num) + "\"\r");
  delay(500);
  
  ATResponse = ATCommand(msg);
  delay(500);
  
  gsmSerial->println((char)26);
  delay(500);

  gsmSerial->println();

  if (debug_mode)
    Serial.println("Text Sent.");
  delay(500);
}
