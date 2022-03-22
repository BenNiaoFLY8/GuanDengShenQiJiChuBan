
/**********************************************************************
项目名称/Project          : 局域网开关灯神器
程序名称/Program name     : LAN_Controller V0.0
作者/Author               : 笨鸟FLY8
日期/Date（YYYYMMDD）     : 20211203
程序目的/Purpose          : 使用ESP8266建立基本服务器,用户可通过浏览器访问8266所建立的网页,并通过该页面控制舵机，打开、关闭灯的开关。
***********************************************************************/

#include <ESP8266WiFi.h>        // 本程序使用 ESP8266WiFi库
#include <ESP8266WebServer.h>   //  ESP8266WebServer库
#include <FS.h>

#include "Config.h"//其他配置信息
//=================配置网络信息=====================//

const char* ssid     = "笨鸟FLY8";      // 连接WiFi名需要连接的WiFi名填入引号中
const char* password = "benniaoFLY8";   // 连接WiFi密码需要连接的WiFi密码填入引号中

//===============为了方便访问，配置为固定IP，需要根据自己路由器调整===================//

IPAddress local_IP(192,168,31,31);    // 设置IP
IPAddress gateway(192,168,31, 1);     // 设置网关
IPAddress subnet(255, 255, 255, 0);   // 设置子网掩码
IPAddress dns(61,134,1,4);            // 设置局域网DNS

ESP8266WebServer esp8266_server(80);// 建立网络服务器对象，该对象用于响应HTTP请求。监听端口（80）

void setup(void)
{
  Serial.begin(115200);   // 启动串口通讯波特率115200

  InitPort();//初始化端口

  // 设置联网参数
  if (!WiFi.config(local_IP, gateway, subnet, dns)) {
    Serial.println("Failed to Config ESP8266 IP"); 
  } 

  WiFi.begin(ssid, password);                  // 启动网络连接
  Serial.print("Connecting to ");              // 串口监视器输出网络连接信息
  Serial.print(ssid); 
  Serial.println(" ...");                      // 告知用户控制器正在尝试WiFi连接

  int i = 0;                                   // 这一段程序语句用于检查WiFi是否连接成功 
  while (WiFi.status() != WL_CONNECTED)        // WiFi.status()函数的返回值是由WiFi连接状态所决定的。 
  {
    delay(1000);                               // 如果WiFi连接成功则返回值为WL_CONNECTED                       
    Serial.print(i++); 
    Serial.print(' ');                         // 此处通过While循环每秒检查一次WiFi.status()函数返回值
  }                                            // 同时通过串口监视器输出连接时长读秒。
                                               // 这个读秒是通过变量i每隔一秒自加1来实现的。
  
  // WiFi连接成功后将通过串口监视器输出连接成功信息 
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // 通过串口监视器输出连接的WiFi名称
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // 通过串口监视器输出控制板的IP

  if(SPIFFS.begin())                       // 启动闪存文件系统
  {
    Serial.println("SPIFFS Started");
  } 
  else 
  {
    Serial.println("SPIFFS Failed to Start");
  }     
 
  esp8266_server.on("/setLED", HandleSwitch); // 设置浏览器访问网站首页时的回调函数
  
  esp8266_server.onNotFound(HandleUserRequest); // 处理其它网络请求

  esp8266_server.begin();                       // 启动网站服务
  Serial.println("Server Started");
}
 
// 设置处理404情况的函数'handleNotFound'
void handleNotFound()
{
  esp8266_server.send(404, "text/plain", "404: Not found"); // 发送"404: Not found"
}

void loop(void) 
{
  esp8266_server.handleClient();//监测网络请求
}

void HandleSwitch(void) //回调函数
{
 String ledState = "OFF";
 String LED_State = esp8266_server.arg("LEDstate"); //获取参数
 Serial.println(LED_State);

 if(LED_State == "1")
 {
  digitalWrite(LED_BUILTIN,LOW); //LED ON
  SG_90_Ctr(true);//打开开关
 } 
 else 
 {
  digitalWrite(LED_BUILTIN,HIGH); //LED OFF
  SG_90_Ctr(false);//关闭开关
 }
}

// 处理用户浏览器的HTTP访问
void HandleUserRequest(void) 
{         
  String reqResource = esp8266_server.uri();  // 获取用户请求资源(Request Resource）
  Serial.print("reqResource: ");
  Serial.println(reqResource);
  
  bool fileReadOK = handleFileRead(reqResource);  // 通过handleFileRead函数处处理用户请求资源

  // 如果在SPIFFS无法找到用户访问的资源，则回复404 (Not Found)
  if(!fileReadOK)
  {                                                 
    esp8266_server.send(404, "text/plain", "404 Not Found"); 
  }
}

bool handleFileRead(String resource)              //处理浏览器HTTP访问
{
  if(resource.endsWith("/"))                      // 如果访问地址以"/"为结尾
  {
    resource = "/index.html";                     // 则将访问地址修改为/index.html便于SPIFFS访问
  } 
  
  String contentType = getContentType(resource);  // 获取文件类型
  
  if (SPIFFS.exists(resource))                    // 如果访问的文件可以在SPIFFS中找到 
  {
    File file = SPIFFS.open(resource, "r");       // 则尝试打开该文件
    esp8266_server.streamFile(file, contentType); // 并且将该文件返回给浏览器
    file.close();                                 // 并且关闭文件
    return true;                                  // 返回true
  }
  return false;                                   // 如果文件未找到，则返回false
}

/**************************************************
 * 函数名称：SG_90_Ctr
 * 函数功能：控制舵机开/关灯
 * 参数说明：Flag：true  打开开关。Flag：false 关闭开关。  
**************************************************/
void SG_90_Ctr(bool Flag)
{
  int k;  
  if(true == Flag)//打开开关
  {
    for(k=0;k<20;k++)
    {
        digitalWrite(SG90_PWM, HIGH);// 输出高
        delayMicroseconds(1800);
        digitalWrite(SG90_PWM, LOW);// 输出低
        delayMicroseconds(19200);
      }
    }
    else//关闭开关
    {
      for(k=0;k<20;k++)
      {
        digitalWrite(SG90_PWM, HIGH);// 输出高
        delayMicroseconds(1200);
        digitalWrite(SG90_PWM, LOW);// 输出低
        delayMicroseconds(18800);
       }
    }

    for(k=0;k<20;k++)//舵机复位
    {
      digitalWrite(SG90_PWM, HIGH);// 输出高
      delayMicroseconds(1500);
      digitalWrite(SG90_PWM, LOW);// 输出低
      delayMicroseconds(18500);
    }
}

// 获取文件类型
String getContentType(String filename){
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

/**************************************************
 * 函数名称：InitPort
 * 函数功能：初始化端口 
 * 参数说明：无
**************************************************/

void InitPort(void)
{
  pinMode(LED_BUILTIN, OUTPUT);   // 初始化板载LED引脚为OUTPUT
  digitalWrite(LED_BUILTIN, HIGH);// 初始化LED引脚状态

  pinMode(KEY, INPUT);   // 初始按键为INPUT

  pinMode(SG90_PWM, OUTPUT);   // 初始化GPIO13引脚为OUTPUT,SG90的PWM控制端口
  digitalWrite(SG90_PWM, LOW); // 初始化为低电平

  pinMode(SG90_POWER, OUTPUT);   // 初始化GPIO12引脚为OUTPUT,SG90的电源控制端口
  digitalWrite(SG90_POWER, HIGH);// 初始化LED引脚状态
}
