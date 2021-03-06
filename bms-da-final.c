/*
  @Author ： zhilong.liu
  @Date : 2014.12.23 update
  @Version : 1.2
  @Memo : 用于楼宇防火监控项目的路由节点的Arduino程序
*/

//Xbee include
#include <XBee.h>
#include <math.h>

//Ethernet include
#include <SPI.h>
#include <Ethernet.h>
#include <stdlib.h>

//screen include
#include <Wire.h>
#include <mpr121.h>
#include "ASCII7x8.h"

//sd include
#include <SD.h>

// ------- Screen begin ---
#define iic_bus 0x3C
#define DC 44
#define RES 45
int currentLine = 0;
// ------- Screen end -----

// ------- ALERT begin ---------
#define LingPin 30
#define SW1Pin 40
int lingState;
// ------- ALERT end -----------

// ------- SD begin ---------
#define SDPin 4
int port;
String targetpath;
String alertpath;
File file;
double _temperature, _co, _flash;
// ------- SD end -----------

// ------- Ethernet begin ---
IPAddress server;
byte mac[] = { 0xCC, 0x52, 0xAF, 0xFE, 0xB2, 0x49 };
EthernetClient cli;
String dhcpip;
// ------- Ethernet end -----

// ------- Xbee begin -------
String serialnum = "";
char lsb[8];
double temperature, co, flash, hum = 50;
XBee xbee = XBee();
ZBRxIoSampleResponse ioSample = ZBRxIoSampleResponse();
XBeeAddress64 test = XBeeAddress64();
// ------- Xbee end ---------

//oled ： 发送命令到i2c总线
void send_cmd(unsigned char COM) {
  Wire.beginTransmission(iic_bus);
  Wire.write(0x00);
  Wire.write(COM);
  Wire.endTransmission();
}

//oled : 发送数据到i2c总线
void send_data(unsigned char DATA) {
  Wire.beginTransmission(iic_bus);
  Wire.write(0x40);
  Wire.write(DATA);
  Wire.endTransmission();
}

//oled : 初始化oled屏幕
void init_oled() {
  digitalWrite(RES, HIGH);   delay(100);
  digitalWrite(RES, LOW);    delay(100);
  digitalWrite(RES, HIGH);   delay(100);
  send_cmd(0XAE);//display off
  send_cmd(0X00);//set lower column address
  send_cmd(0X10);//set higher column address
  send_cmd(0X40);//set display start line
  send_cmd(0XB0);//set page address
  send_cmd(0X81);//set contract control
  send_cmd(0XCF);// VCC Generated by Internal DC/DC Circuit
  send_cmd(0XA1);//set segment remap  column address 127 is mapped to SEG0
  send_cmd(0XA6);//normal / reverse   normal display
  send_cmd(0XA8);//multiplex ratio
  send_cmd(0X3F);//1/64
  send_cmd(0XC8);//Com scan direction remapped mode. Scan from COM[N-1] to COM0
  send_cmd(0XD3);//set display offset
  send_cmd(0X00);
  send_cmd(0XD5);//set osc division
  send_cmd(0X80);
  send_cmd(0XD9);//set pre-charge period
  send_cmd(0X11);
  send_cmd(0XDa);//set COM pins
  send_cmd(0X12);
  send_cmd(0X8d);/*set charge pump enable*/
  send_cmd(0X14);
  send_cmd(0Xdb);//Set VcomH
  send_cmd(0X20);
  send_cmd(0XAF);//display ON
  clean_screen(0x00);
}

//oled ； 清全屏
void clean_screen(unsigned char dat) {
  unsigned char i, j;
  send_cmd(0x00);//set lower column address
  send_cmd(0x10);//set higher column address
  send_cmd(0xB0);//set page address
  for (j = 0; j < 8; j++) {
    send_cmd(0xB0 + j); //set page address
    send_cmd(0x00);//set lower column address
    send_cmd(0x10);//set higher column address
    for (i = 0; i < 128; i++) {
      send_data(dat);
    }
  }
}

//在指定位置写入一个char
//@param line : 行数
//@param page : 行数
void set_ascii7x8(byte line, byte bit, byte ascii) {
  send_cmd(0xB0 + line); //set line address
  send_cmd(0x00 + bit & 0x0f); //set lower column address
  send_cmd(0x10 + bit / 16); //set higher column address
  for (byte i = 0; i < 7; i++)
    send_data(ASCII7x8[ascii * 7 + i]);
}

//打印一行到指定行
//@param input : 输入
//@linenum : 行号
void print2screen(String input, int linenum) {
  int input_Length = input.length() + 1, i = 0;
  char tempchar[input_Length];
  char *p = tempchar;
  input.toCharArray(tempchar, input_Length);
  while (*p != '\0' && i < 122)
  {
    switch (*p) {
      case ' ':
        set_ascii7x8(linenum, i, *p++);
        i += 3;
        break;
      case '.':
        set_ascii7x8(linenum, i, *p++);
        i += 3;
        break;
      case ':':
        set_ascii7x8(linenum, i, *p++);
        i += 3;
        break;
      default :
        set_ascii7x8(linenum, i, *p++);
        i += 6;
    }
  }
}

//从sd卡加载ip地址和端口
void loadConfig() {
  int ip_part1, ip_part2, ip_part3, ip_part4;

  Serial.println("[+] Initializing SD Card.");
  if (!SD.begin(SDPin)) {
    Serial.println("[-] Initialization SD Card Failed!");
    return;
  }

  Serial.println("[+] Initialization SD Card Done.");
  file = SD.open("target.txt");
  String tempstr;
  if (file) {
    while (file.available()) {
      char c = file.read();
      tempstr += c;
    }
  }
  file.close();
  tempstr.trim();
  ip_part1 = tempstr.substring(0, tempstr.indexOf('.')).toInt();
  tempstr = tempstr.substring(tempstr.indexOf('.') + 1);
  ip_part2 = tempstr.substring(0, tempstr.indexOf('.')).toInt();
  tempstr = tempstr.substring(tempstr.indexOf('.') + 1);
  ip_part3 = tempstr.substring(0, tempstr.indexOf('.')).toInt();
  tempstr = tempstr.substring(tempstr.indexOf('.') + 1);
  ip_part4 = tempstr.substring(0, tempstr.indexOf(':')).toInt();
  tempstr = tempstr.substring(tempstr.indexOf(':') + 1);
  port = tempstr.substring(0, tempstr.indexOf('/')).toInt();
  targetpath = tempstr.substring(tempstr.indexOf('/'));

  IPAddress _ip(ip_part1, ip_part2, ip_part3, ip_part4);
  server = _ip;
  Serial.print("[+] Get target ip : ");
  Serial.println(server);
  Serial.print("[+] Get target port : ");
  Serial.println(port);
  Serial.print("[+] Get target path : ");
  Serial.println(targetpath);

  print2screen((String)"+ SERVER:" + (String)ip_part1 + "." + (String)ip_part2 + "." + (String)ip_part3 + "." + (String)ip_part4, 0);

  file = SD.open("alert.txt");
  tempstr = "";
  if (file) {
    while (file.available()) {
      char c = file.read();
      tempstr += c;
    }
  }
  file.close();
  tempstr.trim();

  _temperature = tempstr.substring(0, tempstr.indexOf('.')).toInt();
  tempstr = tempstr.substring(tempstr.indexOf('.') + 1);
  _co = tempstr.substring(0, tempstr.indexOf('/')).toInt();
  alertpath = tempstr.substring(tempstr.indexOf('/')); 
  Serial.print("[+] Get _temperature Red Line : ");
  Serial.println(_temperature);
  Serial.print("[+] Get _co Red Line : ");
  Serial.println(_co);
  Serial.println((String)"[+] Get alertpath : " + (String)alertpath);
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
    (i < 3) ? Serial.print('.') : Serial.println();
  }
  print2screen((String)"+ LOCAL  :" + (String)(Ethernet.localIP()[0]) + "." + (String)(Ethernet.localIP()[1]) + "." + (String)(Ethernet.localIP()[2]) + "." + (String)(Ethernet.localIP()[3]), 1);
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
      ltoa(ioSample.getRemoteAddress64().getLsb(), lsb, 16);
      serialnum = lsb;
      serialnum.toUpperCase();

      //获取温度数据
      temperature = ceil((ioSample.getAnalog(1) * 120 / 1024));

      //获取一氧化碳
      co = ceil((ioSample.getAnalog(2) * 120 / 1024)) + 10;

      //获取光线强度
      flash = ceil((ioSample.getAnalog(3) * 120 / 1024));

      Serial.print(" [-] Data : ");
      Serial.print(" serialnum:");
      Serial.print(serialnum);
      Serial.print(" temperature:");
      Serial.print(temperature);
      Serial.print(" co:");
      Serial.print(co);
      Serial.print(" flash:");
      Serial.println(flash);
      print2screen("-------------------------", 2);
      print2screen((String)"+ CURRENT : [" + (String)serialnum + (String)"] ", 3);
      print2screen((String)"+ T [" + (String)temperature + (String)"]   CO [" + (String)co + "]", 4);
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

//发送数据到api服务器
void dataSender(String targetPath, String postdata) {
  Serial.print("[+]post body : ");
  Serial.println(postdata);
  print2screen((String)"----------------------", 5);
  Serial.println();
  Serial.println("[+] connecting...");
  print2screen((String)"+ NET CONNECTING         ", 6);
  if (cli.connect(server, port)) {
    Serial.println(" [-] connected");
    print2screen((String)"+ NET CONNECTED             ", 6);
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
      while (cli.available())
      {
        char recv = cli.read();
        Serial.print(recv);
      }
    } else {
      Serial.println("[-] no response received.");
      print2screen((String)"+ NO RESPONSE", 6);
    }

    Serial.println();
    Serial.println(" [-] connection over");
    print2screen((String)"+ SEND SUCCESS", 6);
  } else {
    Serial.println(" [-] connection failure.");
    print2screen((String)"+ SEND FAILED", 6);
  }

  cli.stop();
  delay(5000);
}

void ling()
{
  lingState = 0;
  digitalWrite(LingPin,lingState);
}

//构造普通的post请求数据
String generateNormalPost() {
  String postdata = "temperature=";
  postdata += temperature;
  postdata += "&humidity=";
  postdata += 50;
  postdata += "&co=";
  postdata += co;
  postdata += "&smoke=";
  postdata += flash;
  postdata += "&serialnum=";
  postdata += serialnum;
  return postdata;
}

//检查报警状态
void checkFire(String serialnum, double temperature, double co, double flash) {
  if (temperature > _temperature && co > _co && digitalRead(SW1Pin)==HIGH) {
      print2screen((String)"= [ALERT] : [" + (String)serialnum + "]", 7);
      lingState = 1;
      digitalWrite(LingPin,lingState);
      String alertpostbody = "serialnum=";
      alertpostbody += serialnum;
      dataSender(alertpath, alertpostbody);
  }
}

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);

  // Screen start
  pinMode(RES, OUTPUT); //RES
  pinMode(DC, OUTPUT); //D/C#
  pinMode(LingPin, OUTPUT); //LingLingLing
  pinMode(SW1Pin,INPUT);

  lingState = digitalRead(SW1Pin);
  attachInterrupt(SW1Pin,ling,CHANGE);

  digitalWrite(DC, LOW);
  Wire.begin();
  init_oled();
  mpr121Init();
  analogReadResolution(12);
  delay(50);

  //加载服务器ip地址
  loadConfig();
  //dhcp获取本机ip
  DHCP();
  print2screen((String)"= [ALERT] : [NONE]", 7);
}

void loop()
{
  xbeeReader();
  checkFire(serialnum, temperature, co, flash);
  dataSender(targetpath, generateNormalPost());
}