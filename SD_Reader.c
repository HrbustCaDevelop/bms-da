#include <SPI.h>
#include <SD.h>
#include<Ethernet.h>

int SDPin = 4;
int port;
IPAddress ip;
File file;

void setup() {
  Serial.begin(9600);
  loadIP(ip, port);
}

//从sd卡加载ip地址和端口
void loadIP(IPAddress &ip, int &port) {

  int ip_part1, ip_part2, ip_part3, ip_part4;

  Serial.println("[+] Initializing SD Card");
  pinMode(10, OUTPUT);

  if (!SD.begin(SDPin)) {
    Serial.println("[+] Initialization Failed!");
    return;
  }

  Serial.println("[+] Initialization Done.");
  file = SD.open("ip.txt");
  String tempstr;
  if (file) {
    while (file.available()) {
      char c = file.read();
      tempstr+=c;
    }
  }

  file.close();
  tempstr.trim();
  ip_part1 = tempstr.substring(0,tempstr.indexOf('.')).toInt();
  tempstr = tempstr.substring(tempstr.indexOf('.')+1);
  ip_part2 = tempstr.substring(0,tempstr.indexOf('.')).toInt();
  tempstr = tempstr.substring(tempstr.indexOf('.')+1);
  ip_part3 = tempstr.substring(0,tempstr.indexOf('.')).toInt();
  tempstr = tempstr.substring(tempstr.indexOf('.')+1);
  ip_part4 = tempstr.substring(0,tempstr.indexOf(':')).toInt();
  port = tempstr.substring(tempstr.indexOf(':')+1).toInt(); 
  IPAddress _ip(ip_part1,ip_part2,ip_part3,ip_part4);
  ip = _ip;
  Serial.println(ip);
  Serial.println(port);
}

void loop() {
  
}