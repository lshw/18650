#define IC1 A4 //Vref R0=0.1 1号电池充电电流 内置基准电压1.1V 采样电阻1.1/3欧姆  满量程3A分辨率2.929688ma
#define IC2 A1 //Vref R0=0.1 2号电池充电电流
#define IC3 A0 //Vref R0=0.1 3号电池充电电流
#define IC4 A7 //Vref R0=0.1 4号电池充电电流
#define IC5 A5 //Vref R0=0.1 5号电池充电电流
#define IF1 A3 //Vref R0=0.1 1号电池放电电流
#define VCC A2 //Vref=1.1  V5=1.1*VCC/1024/24*(24+499); //外接电源电压
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

float getval(int VIN)   //读取电流电压值  电流为ma ，电压为V
{
  uint16_t i;  
  float val;
  switch(VIN) {
  case VCC:
    val=1.1*(24+499)/24/1024;  
    break;
  case V1:
    val=1.1*(97.6+499)/97.6/1024;                       
    break;
  default:
    val=1.1/(1.1/3)*1000/1024;    //1000->换算成ma, 1024->10位AD, 1.1/3->取样电阻
  }
  i=analogRead(VIN);
  return(val*i);
}
void setup()
{
  Serial.begin(9600);
  Serial.println("hello,Cfido!");
analogReference(INTERNAL); //1.1V 基准
analogRead(A0);
Serial.print("tf=");
Serial.println(sdInit());
lcd.begin(16, 2);
lcd.clear();
lcd.print("hello, Cfido!");
}
  char charVal[15]; 
  float floatVal;

void loop() 
{
  String stringVal ;
   poweroff();
  floatVal=getval(VCC);
  dtostrf(floatVal, 4, 2, charVal);  //4 is mininum width, 3 is precision; float value is copied onto buff
  stringVal="VIN,"+String(charVal)+"V";
  sdSave(stringVal+"\r\n");
  Serial.println(stringVal);
    lcd.setCursor(0, 0);
    lcd.print(charVal);
  Discharge();
  floatVal=getval(V1);
  //charge();
  dtostrf(floatVal, 4, 2, charVal);  //4 is mininum width, 3 is precision; float value is copied onto buff
  stringVal="V1,"+String(charVal)+"V\r\n";
  sdSave(stringVal);
  Serial.print(stringVal);
  lcd.setCursor(5, 0);
    lcd.print(String(charVal));
    
    floatVal=getval(IF1);
  dtostrf(floatVal, 4, 0, charVal);  //4 is mininum width, 3 is precision; float value is copied onto buff
  stringVal="IF1,"+String(charVal)+"ma\r\n";
  sdSave(stringVal);
  Serial.print(stringVal);
  lcd.setCursor(10, 0);
    lcd.print(String(charVal));
    
  floatVal=getval(IC1);
  dtostrf(floatVal, 4, 0, charVal);  //4 is mininum width, 3 is precision; float value is copied onto buff
  stringVal="IC1,"+String(charVal)+"ma\r\n";
  sdSave(stringVal);
  Serial.print(stringVal);
  lcd.setCursor(10, 0);
    lcd.print(String(charVal));
  
  floatVal=getval(IC2);
  dtostrf(floatVal, 4, 0, charVal);  //4 is mininum width, 3 is precision; float value is copied onto buff
  stringVal="IC2,"+String(charVal)+"ma\r\n";
  sdSave(stringVal);
  Serial.print(stringVal);
    lcd.setCursor(0, 1);
    lcd.print(String(charVal));
  
  floatVal=getval(IC3);
  dtostrf(floatVal, 4, 0, charVal);  //4 is mininum width, 3 is precision; float value is copied onto buff
  stringVal="IC3,"+String(charVal)+"ma\r\n";
  sdSave(stringVal);
  Serial.print(stringVal);
    lcd.setCursor(4, 1);
    lcd.print(String(charVal));
  
  floatVal=getval(IC4);
  dtostrf(floatVal, 4, 0, charVal);  //4 is mininum width, 3 is precision; float value is copied onto buff
  stringVal="IC4,"+String(charVal)+"ma\r\n";
  sdSave(stringVal);
  Serial.print(stringVal);
 
   lcd.setCursor(8, 1);
    lcd.print(String(charVal));
  
  floatVal=getval(IC5);
  dtostrf(floatVal, 4, 0, charVal);  //4 is mininum width, 3 is precision; float value is copied onto buff
  stringVal="IC5,"+String(charVal)+"ma\r\n";
  sdSave(stringVal);
  Serial.print(stringVal);
  
    lcd.setCursor(12, 1);
    lcd.print(String(charVal));
  
delay(2000);
}

