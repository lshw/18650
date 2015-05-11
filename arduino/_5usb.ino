#define IC1 A7 //Vref R0=0.1 1号电池充电电流 内置基准电压1.1V 采样电阻1/3欧姆  满量程3.3A分辨率3.222656ma
#define IC2 A0 //Vref R0=0.1 2号电池充电电流
#define IC3 A1 //Vref R0=0.1 3号电池充电电流
#define IC4 A2 //Vref R0=0.1 4号电池充电电流
#define IC5 A3 //Vref R0=0.1 5号电池充电电流
#define IF1 A5 //Vref R0=0.1 1号电池放电电流
#define VCC A4 //Vref=1.1  V5=1.1*VCC/1024/22*(22+499); //外接电源电压
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
digitalWrite(2,LOw);  //pin2 拉低
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

float getval(int VIN)   //读取电流电压值  电流为ma ，电压为V
{
  uint16_t i;  
  float val;
  switch(VIN) {
  case VCC:
    val=1.1*(22+499)/22/1024;  
    break;
  case V1:
    val=1.1*(97.6+499)/97.6/1024;                       
    break;
  default:
    val=1.1*3*1000/1024;    //1000->换算成ma, 1024->10位AD, 0.33333333->取样电阻
  }
  i=analogRead(VIN);
  return(val*i);
}
void setup()
{
analogReference(INTERNAL); //1.1V 基准
analogRead(A0);
sdInit();
lcd.begin(16, 2);
lcd.print("hello, Cfido!");
}
  char charVal[15]; 
  float floatVal;

void loop() 
{
  String stringVal ;
  floatVal=getval(VCC);
  dtostrf(floatVal, 4, 3, charVal);  //4 is mininum width, 3 is precision; float value is copied onto buff
  stringVal="VIN,"+String(charVal)+"\r\n";
  sdSave(stringVal);
  Serial.print(stringVal);

  floatVal=getval(IC1);
  dtostrf(floatVal, 4, 3, charVal);  //4 is mininum width, 3 is precision; float value is copied onto buff
  stringVal="IC1,"+String(charVal)+"\r\n";
  sdSave(stringVal);
  Serial.print(stringVal);

  floatVal=getval(IC2);
  dtostrf(floatVal, 4, 3, charVal);  //4 is mininum width, 3 is precision; float value is copied onto buff
  stringVal="IC2,"+String(charVal)+"\r\n";
  sdSave(stringVal);
  Serial.print(stringVal);
delay(600000);
}

