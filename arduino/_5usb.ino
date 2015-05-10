#define VREF 3.3*68/(2000+68) //外部参考电压
#define IC1 A7 //Vref R0=0.1 1号电池充电电流
#define IC2 A0 //Vref R0=0.1 2号电池充电电流
#define IC3 A1 //Vref R0=0.1 3号电池充电电流
#define IC4 A2 //Vref R0=0.1 4号电池充电电流
#define IC5 A3 //Vref R0=0.1 5号电池充电电流
#define IF1 A5 //Vref R0=0.1 1号电池放电电流
#define VCC A4 //Vref=3.3  V5=3.3*VCC/1024/49.9*(49.9+499); //外接电源电压
#define V1 A6  //Vref=3.3 3.3*V1/1024/2  //1号电池电压
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
  __volatile__ uint16_t i;  //不做GCC优化，否则第一次AD会被优化掉
  float val;
  switch(VIN) {
  case VCC:
    analogReference(DEFAULT); //3.3V
    val=3.3*(49.9+499)/49.9/1024;  //外接分压电阻
    break;
  case V1:
    analogReference(DEFAULT); //3.3V
    val=3.3*2/1024;                       
    break;
  default:
    analogReference(EXTERNAL);  //3.3V*68/(2000+68)
    val=1000*VREF/1024/0.1;    //1000->换算成ma，VREF->外接基准, 1024->10位AD, 0.1->0.1欧姆取样电阻
  }
  i=analogRead(VIN); //第一次转换是无效数值
  i=analogRead(VIN);
  val=val*i;
  return(val);
}
void setup()
{
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

