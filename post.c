#include <SPI.h>
#include <Ethernet.h>

// 指定mac地址
byte mac[] = {  0xB8, 0x27, 0xEB, 0xFE, 0xB2, 0x49 };
IPAddress server(222, 27, 196, 5);
// 创建一个EthernetClient实例
EthernetClient cli;

void setup() 
{
  Serial.begin(9600);
  // DHCP 初始化
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("[+] DHCP configuration fail.");
    // 如果DHCP分配失败则永远暂停在这里，否则可能会有意想不到的事发生。
    for(;;);
  }
  Serial.print("[+] DHCP configuration successful. \n[+] IP : ");
  for (byte i = 0; i < 4; i++)
  {
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
  if (cli.connect(server, 80))
  {
    Serial.println("[+] connected...");
    // 格式化HTTP头，可以添加useragent等，但是不能删除现有的，否则会提交失败
    Serial.println("[+] forming & send HTTP request message");
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
    // 得到回复，如果服务端还在链接的话。
//    if (cli.available())
//    {
//      while(cli.available())
//      {
//        char recv = cli.read();
//        Serial.print(recv);
//      }
//    } else {
//      Serial.println("[+] no response received.");
//    }
    cli.stop();
    Serial.println("[+] connection termination.");
  } else {
    Serial.println("[+] connection failure.");
  }
  // 这里等待八秒，整个程序一共等待10秒。
  delay(8000);
}