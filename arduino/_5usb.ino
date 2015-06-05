//使用arduino环境，通过usb串口,下载程序到充电器,板卡类型选择arduino uno，
//下载开始的瞬间,要按一下充电器反面的复位键
#define IC1 A4 //Vref R0=0.1 1号电池充电电流 内置基准电压1.1V 采样电阻0.33欧姆  满量程3.33A分辨率3.255208ma
#define IC2 A1 //Vref R0=0.1 2号电池充电电流
#define IC3 A0 //Vref R0=0.1 3号电池充电电流
#define IC4 A7 //Vref R0=0.1 4号电池充电电流
#define IC5 A5 //Vref R0=0.1 5号电池充电电流
#define IF1 A3 //Vref R0=0.1 1号电池放电电流
#define VCC A2 //Vref=1.1  1.1*VCC/1024/24.3*(24.3+499); //外接电源电压
#define V1 A6  //Vref=1.1  1.1*V1/1024/97.6*(97.6+499)  //1号电池电压  
#include<stdlib.h>
#include <LiquidCrystal.h>   //LCD1602a 驱动
LiquidCrystal lcd(8, 7, 6, 5, 4, 3); //(RS,EN,D4,D5,D6,D7)

#include <SD.h>  //sdcard 和vfat的库
boolean sdInit()
{
  pinMode(10, OUTPUT);
  return(SD.begin(10));   //10 is CS
}
void sdSave(String dataString) {
  File dataFile = SD.open("datalog.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
  }
}
void charge()
{ //电池1充电
  pinMode(9,INPUT);
  digitalWrite(9,LOW); //pin9 高阻
  pinMode(2,OUTPUT);   
  digitalWrite(2,LOW);  //pin2 拉低
}
void Discharge()
{ //电池1放电
  pinMode(9,OUTPUT);
  digitalWrite(9,HIGH);  //pin9 拉高
  pinMode(2,OUTPUT);
  digitalWrite(2,HIGH);   //pin2 拉高
}
void disable()
{ //电池1不充不放
  pinMode(9,OUTPUT);
  digitalWrite(9,HIGH);  //pin9 拉高
  pinMode(2,OUTPUT);
  digitalWrite(2,LOW);   //pin2 拉低
}

uint16_t getval(int VIN)   //读取电流电压值  电流为ma ，电压为mV数/10
{
  uint32_t val; //每个数字都乘以100倍,是为了保留足够的小数位数
  switch(VIN) {
  case VCC:  
    val=1.1*1000*100*(24.3+499)/24.3/1024;  
    break;
  case V1:   //10mv
    val=1.1*1000*100*(97.6+499)/97.6/1024;                       
    break;
  default:  //ma
    val=1.1*1000*100/(0.33)/1024;    //1000->换算成ma, 1024->10位AD, 0.33->取样电阻
  }
  return(val*analogRead(VIN)/100); //调整倍数，
}
String getma(uint16_t a) 
{  //毫安数是3位的， 前面要补0,
  String c;
  if(a>999) a=999; //最大999ma
  if(a<10) c="0";  //补2个0
  if(a<100) c+="0"; //补1个0
  return c+String(a);
}
String getmv(uint16_t v)
{  //电压最多取2位小数，前面补0
  v=v%100;
  if(v<10) return "0"+String(v);
  else return String(v);
}
void setup()
{
  String hello="Hello,Cfido!";
  Serial.begin(9600); //串口9600
  Serial.println(hello);
  analogReference(INTERNAL); //使用atmega328的内部1.1V 基准源
  analogRead(A0);
  lcd.begin(16, 2); //lcd初始化  16字符2行
  lcd.clear();
  lcd.print(hello); 
  pinMode(11,OUTPUT);
  for(int a=0;a<10;a++) {
  delay(100); 
  digitalWrite(11,!digitalRead(11)); //lcd背光煽动10次
  }
  pinMode(11,INPUT);  //这个腿要作为tf卡的片选，所以要改成输入
  Serial.print("sdcard=");      
if(sdInit())  //tf初始化测试
  Serial.println("ok!");
  else 
  Serial.println("Error!");
  
}

void loop() 
{ //循环
  String stringVal ; //显示buff
  uint16_t va; //存放模拟量的值
  disable(); // 关闭1号电池的充放电，
  va=getval(VCC); //测电源电压
  stringVal=String(va/100)+"."+String(va%100/10);  //换算成字符串 
  va=getval(V1); //测一号电池的电压， 关闭充放电的情况下测试
  charge(); //1号电池测完，测完打开到充电模式
  stringVal +="V "+String(va/100)+"."+getmv(va%100)+"V "; //换算成字符串
  va=getval(IF1); //放电电流
  if(va>0) 
    stringVal +="-"+getma(va)+" "; //如果不是0,则作为负值放入显示buff

  va=getval(IC1); //测充电电流
  if(va>0)
    stringVal += getma(va)+"ma"; //放入显示buff
  Serial.println(stringVal); //把显示buff送串口
  lcd.setCursor(0, 0); //设置光标到第一行第一个字符位置
  lcd.print(stringVal);  //显示字符串到第一行
  sdSave(stringVal+"\r\n");  //保存字符串到sd卡
//开始准备第二行
  stringVal = getma(getval(IC2))+" "; //把2号电流放入buff，3位数字加一个空格 
  stringVal+=getma(getval(IC3))+" "; //把3号电流放入buff，3位数字加一个空格
  stringVal+=getma(getval(IC4))+" "; //把4号电流放入buff，3位数字加一个空格
  stringVal+=getma(getval(IC5))+" "; //把5号电流放入buff，3位数字加一个空格
//第二行准备完毕
  sdSave(stringVal+"\r\n"); //存入sdcard
  Serial.print(stringVal+"\r\n"); //输出到串口
  lcd.setCursor(0, 1);  //设置光标位置到第二行的左边
  lcd.print(stringVal);   //显示buff到第二行
  delay(1000); //延迟一秒后继续
}
