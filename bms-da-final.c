#include <SPI.h>
#include <SD.h>
#include <stdlib.h>
#include <XBee.h>
#include <math.h>
#include <Ethernet.h>

// ------- SD begin ---------
int SDPin = 4;
int port;
String targetpath;
File file;
// ------- SD end -----------

// ------- Ethernet begin ---
IPAddress server;
byte mac[] = { 0xB8, 0x27, 0xEB, 0xFE, 0xB2, 0x49 };
EthernetClient cli;
// ------- Ethernet end -----

// ------- Xbee begin -------
String serialnum ="";
char lsb[8];
double temperature,co,flash,hum = 50;
XBee xbee = XBee();
ZBRxIoSampleResponse ioSample = ZBRxIoSampleResponse();
XBeeAddress64 test = XBeeAddress64();
// ------- Xbee end ---------

//从sd卡加载ip地址和端口
void loadIP() {
  int ip_part1, ip_part2, ip_part3, ip_part4;

  Serial.println("[+] Initializing SD Card.");

  if (!SD.begin(SDPin)) {
    Serial.println(" [-] Initialization SD Card Failed!");
    return;
  }

  Serial.println(" [-] Initialization SD Card Done.");
  file = SD.open("target.txt");
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
  ip_part4 = tempstr.substring(0, tempstr.indexOf(':')).toInt();
  tempstr = tempstr.substring(tempstr.indexOf(':')+1);
  port = tempstr.substring(0, tempstr.indexOf('/')).toInt(); 
  targetpath = tempstr.substring(tempstr.indexOf('/')); 

  IPAddress _ip(ip_part1,ip_part2,ip_part3,ip_part4);
  server = _ip;
  Serial.print(" [-] Get target ip : ");
  Serial.println(server);
  Serial.print(" [-] Get target port : ");
  Serial.println(port);
  Serial.print(" [-] Get target path : ");
  Serial.println(targetpath);
}

//DHCP初始化网卡ip
void DHCP() {
  Serial.println();
  Serial.println("[+] DHCP configuration start...");
  while (Ethernet.begin(mac) != 1) {
    Serial.println(" [-] DHCP configuration fail.");
    delay(1000);
  }
  Serial.print(" [-] DHCP configuration successful. \n [-] IP : ");
  for (byte i = 0; i < 4; i++) {
    Serial.print(Ethernet.localIP()[i], DEC);
    (i < 3) ? Serial.print('.'):Serial.println();
  }
  delay(1000);
}

//通过Xbee读取传感器数据
void xbeeReader() {
  xbee.readPacket();
  
  if (xbee.getResponse().isAvailable()) {
    if (xbee.getResponse().getApiId() == ZB_IO_SAMPLE_RESPONSE) {
      xbee.getResponse().getZBRxIoSampleResponse(ioSample);

      Serial.println();
      Serial.println("[+] Received I/O Sample.");

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

      Serial.print(" [-] Data : ");
      Serial.print(" serialnum:");
      Serial.print(serialnum);
      Serial.print(" temperature:");
      Serial.print(temperature);
      Serial.print(" co:");
      Serial.print(co);
      Serial.print(" flash:");
      Serial.println(flash);
    } 
    else {
      Serial.print(" [-] Expected I/O Sample, because : ");
      Serial.print(xbee.getResponse().getApiId(), HEX);
    }    
  } 
  else if (xbee.getResponse().isError()) {
    Serial.print(" [-] Error reading packet. Error code: ");  
    Serial.println(xbee.getResponse().getErrorCode());
  }
}

void dataSender(String targetPath, String serialnum, double temp, double co, double flash, double hum = 50) {
  //构造post参数
  String postdata = "temperature=";
  postdata+=temp;
  postdata+="&humidity=";
  postdata+=hum;
  postdata+="&co=";
  postdata+=co;
  postdata+="&smoke=";
  postdata+=flash;
  postdata+="&serialnum=";
  postdata+=serialnum;
  Serial.print("[+]post body : ");
  Serial.println(postdata);

  Serial.println();
  Serial.println("[+] connecting...");
  if (cli.connect(server, port)) {
    Serial.println(" [-] connected...");
    // 格式化HTTP头
    Serial.println(" [-] forming & send HTTP request message");
    // post到指定的页面
    cli.print("POST ");
    cli.print(targetPath);
    cli.println(" HTTP/1.1");
    cli.println("Host: hrbust.edu.cn");
    cli.println("Content-Type: application/x-www-form-urlencoded");
    cli.println("Connection: close");
    cli.print("Content-Length: ");
    cli.println(postdata.length());
    cli.println();
    cli.print(postdata);
    cli.println();
    delay(1000);

    // 得到回复，如果服务端还在链接的话。
    if (cli.available())
    {
      while(cli.available())
      {
        char recv = cli.read();
        Serial.print(recv);
      }
    } else {
      Serial.println("[-] no response received.");
    }

    Serial.println();
    Serial.println(" [-] connection over.");
  } else {
    Serial.println(" [-] connection failure.");
  }

  cli.stop();
  delay(5000);
}

void setup() 
{
  Serial.begin(9600);
  Serial1.begin(9600);

  loadIP();
  DHCP();
}

void loop()
{
  xbeeReader();
  dataSender(targetpath, serialnum, temperature, co, flash, hum);
}