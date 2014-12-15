#include <SPI.h>
#include <SD.h>
#include <Ethernet.h>

int SDPin = 4;
int port;

File file;
IPAddress server;
byte mac[] = { 0xB8, 0x27, 0xEB, 0xFE, 0xB2, 0x49 };
EthernetClient cli;

void setup() 
{
  Serial.begin(9600);

  loadIP(server, port);

  Serial.println("[+] DHCP configuration start...");
  // DHCP 初始化
  while (Ethernet.begin(mac) != 1) {
    Serial.println(" [-] DHCP configuration fail.");
    delay(1000);
  }
  Serial.print(" [-] DHCP configuration successful. \n [-] IP : ");
  for (byte i = 0; i < 4; i++) {
    // 打印IP地址:
    Serial.print(Ethernet.localIP()[i], DEC);
    (i < 3) ? Serial.print('.'):Serial.println();
  }
  // 这里必须暂停等待所有都初始化完毕。
  delay(1000);
}

void loop()
{
  Serial.println();
  Serial.println("[+] connecting...");
  // HTTP报文准备的，保存post参数，
  String hcontent = "username=hanzai";
  
  // 尝试链接到服务器
  if (cli.connect(server, port))
  {
    Serial.println(" [-] connected...");
    // 格式化HTTP头，可以添加useragent等，但是不能删除现有的，否则会提交失败
    Serial.println(" [-] forming & send HTTP request message");
    // post到指定的页面
    cli.println("POST /bms/user/checkusername HTTP/1.1");
    cli.println("Host: hrbust.edu.cn");
    cli.println("Content-Type: application/x-www-form-urlencoded");
    cli.println("Connection: close");
    cli.print("Content-Length: ");
    cli.println(hcontent.length());
    cli.println();
    cli.print(hcontent);
    cli.println();
    // 等待1秒来得到服务器的回复
    // 这里如果不等待的话下面的cli.available()还没得到对方服务就会认为对方没有发送返回信息
    delay(1000);
    cli.stop();
    Serial.println(" [-] connection termination success.");
  } else {
    Serial.println(" [-] connection failure.");
  }
  delay(5000);
}

//从sd卡加载ip地址和端口
void loadIP(IPAddress &ip, int &port) {
  int ip_part1, ip_part2, ip_part3, ip_part4;

  Serial.println("[+] Initializing SD Card.");

  if (!SD.begin(SDPin)) {
    Serial.println(" [-] Initialization SD Card Failed!");
    return;
  }

  Serial.println(" [-] Initialization SD Card Done.");
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
  Serial.print(" [-] Get target ip : ");
  Serial.println(ip);
  Serial.print(" [-] Get target port : ");
  Serial.println(port);
}