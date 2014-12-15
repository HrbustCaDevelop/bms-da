#include <stdlib.h>
#include <XBee.h>
#include <math.h>

XBee xbee = XBee();
ZBRxIoSampleResponse ioSample = ZBRxIoSampleResponse();
XBeeAddress64 test = XBeeAddress64();

char lsb[8];
String serialnum ="";
double temperature,co,flash,hum = 50;

void setup() { 
  Serial.begin(9600);
  Serial1.begin(9600);
}

void loop() {  
  xbee.readPacket();
  
  if (xbee.getResponse().isAvailable()) {
    if (xbee.getResponse().getApiId() == ZB_IO_SAMPLE_RESPONSE) {
      xbee.getResponse().getZBRxIoSampleResponse(ioSample);

      Serial.println();
      Serial.println("[+]Received I/O Sample.");

      //获取SN
      ltoa(ioSample.getRemoteAddress64().getLsb(),lsb,16);
      serialnum = lsb;
      serialnum.toUpperCase();

      //获取温度数据
      temperature = ceil((ioSample.getAnalog(1)*120/1024));

      //获取一氧化碳
      co = ceil((ioSample.getAnalog(2)*120/1024)) + 10;
      
      //获取光线强度
      flash = ceil((ioSample.getAnalog(3)*120/1024));

      Serial.print("serialnum:");
      Serial.print(serialnum);
      Serial.print(" temperature:");
      Serial.print(temperature);
      Serial.print(" co:");
      Serial.print(co);
      Serial.print(" flash:");
      Serial.print(flash);
    } 
    else {
      Serial.print(" [-]Expected I/O Sample, because : ");
      Serial.print(xbee.getResponse().getApiId(), HEX);
    }    
  } 
  else if (xbee.getResponse().isError()) {
    Serial.print(" [-]Error reading packet. Error code: ");  
    Serial.println(xbee.getResponse().getErrorCode());
  }
}