#include <stdlib.h>
#include <XBee.h>
#include <math.h>
int RES = 45;
int DC = 44;

XBee xbee = XBee();
ZBRxIoSampleResponse ioSample = ZBRxIoSampleResponse();
XBeeAddress64 test = XBeeAddress64();

long int iaddr=0;
long int liaddr;
char addr[32]={0,}; 
int  iAnalog=0;
int liAnalog=0;
char Analog[8]={0,};
int Digital=0;
int lDigital=0;

void setup() { 
  Serial.begin(9600);
  Serial1.begin(9600);
  
  pinMode(RES,OUTPUT);//RES
  pinMode(DC,OUTPUT);//D/C#
  digitalWrite(DC,LOW);

  delay(3000);
   digitalWrite(RES,HIGH);   
    delay(100);
    digitalWrite(RES,LOW);    
    delay(100);
    digitalWrite(RES,HIGH);   
    delay(100);	 
}

void loop() {
  //attempt to read a packet    
  xbee.readPacket();
  
  if (xbee.getResponse().isAvailable()) {
    // got something
    Serial.println("xbee.getResponse().isAvailable()");
    if (xbee.getResponse().getApiId() == ZB_IO_SAMPLE_RESPONSE) {
      xbee.getResponse().getZBRxIoSampleResponse(ioSample);

      Serial.print("Received I/O Sample from: ");

      Serial.print(ioSample.getRemoteAddress64().getMsb(), HEX);  
      Serial.print(ioSample.getRemoteAddress64().getLsb(), HEX);
      liaddr=ioSample.getRemoteAddress64().getLsb();
      sprintf(addr,"%X",liaddr);         
      Serial.println("");


      if (ioSample.containsAnalog()) {
        Serial.println("Sample contains analog data");
      }

      if (ioSample.containsDigital()) {
        Serial.println("Sample contains digtal data");
      }      

      // read analog inputs
        Serial.println("wendu:");
        Serial.println(ceil((ioSample.getAnalog(1)*120/1024)));
      
        Serial.println("co:");
        Serial.println(ceil((ioSample.getAnalog(2)*120/1024)));
        
        Serial.println("flash:");
        Serial.println(ceil((ioSample.getAnalog(3)*120/1024)));
      

      // check digital inputs
      for (int i = 0; i <= 12; i++) {
        if (ioSample.isDigitalEnabled(i)) {
          Serial.print("Digital (DI");
          Serial.print(i, DEC);
          Serial.print(") is ");
          Serial.println(ioSample.isDigitalOn(i), DEC);
          lDigital=ioSample.isDigitalOn(i);
        }
      }
    } 
    else {
      Serial.print("Expected I/O Sample, but got ");
      Serial.print(xbee.getResponse().getApiId(), HEX);
    }    
  } 
  else if (xbee.getResponse().isError()) {
    Serial.print("Error reading packet.  Error code: ");  
    Serial.println(xbee.getResponse().getErrorCode());
  }
}


