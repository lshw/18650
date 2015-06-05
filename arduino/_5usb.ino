#define IC1 A4 //Vref R0=0.1 1号电池充电电流 内置基准电压1.1V 采样电阻1.1/3欧姆  满量程3A分辨率2.929688ma
#define IC2 A1 //Vref R0=0.1 2号电池充电电流
#define IC3 A0 //Vref R0=0.1 3号电池充电电流
#define IC4 A7 //Vref R0=0.1 4号电池充电电流
#define IC5 A5 //Vref R0=0.1 5号电池充电电流
#define IF1 A3 //Vref R0=0.1 1号电池放电电流
#define VCC A2 //Vref=1.1  V5=1.1*VCC/1024/24.3*(24.3+499); //外接电源电压
#define V1 A6  //Vref=1.1 1.1*V1/1024/97.6*(97.6+499)  //1号电池电压  
#include<stdlib.h>
#include <LiquidCrystal.h>   //LCD1602a 驱动
LiquidCrystal lcd(8, 7, 6, 5, 4, 3); //(RS,EN,D4,D5,D6,D7)

#include <SD.h>
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
void poweroff()
{ //电池1不充不放
  pinMode(9,OUTPUT);
  digitalWrite(9,HIGH);  //pin9 拉高
  pinMode(2,OUTPUT);
  digitalWrite(2,LOW);   //pin2 拉低
}

uint16_t getval(int VIN)   //读取电流电压值  电流为ma ，电压为V
{
  uint32_t val;
  switch(VIN) {
  case VCC:  //10mv
    val=1.1*10000*(24.3+499)/24.3/1024;  
    break;
  case V1:   //10mv
    val=1.1*10000*(97.6+499)/97.6/1024;                       
    break;
  default:  //ma
    val=1.1*100000/(0.33)/1024;    //1000->换算成ma, 1024->10位AD, 0.33->取样电阻
  }
  return(val*analogRead(VIN)/100);
}
String getma(uint16_t a) 
{
  String c;
  if(a>999) a=999;
  if(a<10) c="0";
  if(a<100) c+="0";
  return c+String(a);
}
String getmv(uint16_t v)
{
  v=v%100;
  if(v<10) return "0"+String(v);
  else return String(v);
}
void setup()
{
  String hello="Hello,Cfido!
  ";
  Serial.begin(9600);
  Serial.println(hello);
  analogReference(INTERNAL); //1.1V 基准
  analogRead(A0);
  Serial.print("tf=");
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print(hello);
  pinMode(11,OUTPUT);
  for(int a=0;a<10;a++) {
  delay(100);
  digitalWrite(11,!digitalRead(11));
  }
  pinMode(11,INPUT);
if(sdInit()) 
  Serial.println("ok!");
  else 
  Serial.println("Error!");
  
}

void loop() 
{
  String stringVal ;
  uint16_t va;
  poweroff();
  va=getval(VCC);
  stringVal=String(va/100)+"."+String(va%100/10);  
  va=getval(V1);
  charge();
  stringVal +="V "+String(va/100)+"."+getmv(va%100)+"V ";
  va=getval(IF1);
  if(va>0) stringVal +="-"+getma(va)+" ";

  va=getval(IC1);
  if(va>0)
    stringVal += getma(va)+"ma";
  Serial.print(stringVal);
  lcd.setCursor(0, 0);
  lcd.print(stringVal);
  sdSave(stringVal+"\r\n");
  stringVal = getma(getval(IC2))+" ";
  stringVal+=getma(getval(IC3))+" ";
  stringVal+=getma(getval(IC4))+" ";
  stringVal+=getma(getval(IC5))+" ";
  sdSave(stringVal+"\r\n");
  Serial.print(stringVal+"\r\n");
  lcd.setCursor(0, 1);
  lcd.print(stringVal);  
  delay(1000);
}

