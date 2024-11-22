#ifndef GSMSIM800L_h
#define GSMSIM800L_h

#include <Arduino.h>

class GSMSIM800L {
  
  private:
    Stream *gsmSerial; // Use Stream to support both HardwareSerial and SoftwareSerial
    String ATResponse = "";
    bool debug_mode = true;
    String SMSBuffer = "";
    
  public:
    GSMSIM800L(Stream *serial);
    void init();
    void setDebugging(bool mode);
    String monitorGSMSerial();
    void SendSMS(String msg, String num);
    String ATCommand(String cmd);

    bool receivedSMS = false;
    String senderNumber = "";

    String getReceivedSMS();
};

#endif
