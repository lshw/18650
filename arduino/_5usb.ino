#define VER "1.0"
#define IC1 A4 //Vref R0=0.1 1号电池充电电流 内置基准电压1.1V 采样电阻0.33欧姆  满量程3.33A分辨率3.255208ma
#define IC2 A1 //Vref R0=0.1 2号电池充电电流
#define IC3 A0 //Vref R0=0.1 3号电池充电电流
#define IC4 A7 //Vref R0=0.1 4号电池充电电流
#define IC5 A5 //Vref R0=0.1 5号电池充电电流
#define IF1 A3 //Vref R0=0.1 1号电池放电电流
#define VCC A2 //Vref=1.1  1.1*VCC/1024/24.3*(24.3+499); //外接电源电压
#define V1 A6  //Vref=1.1  1.1*V1/1024/97.6*(97.6+499)  //1号电池电压  
uint16_t ic3,ic2,vcc,if1,ic1,ic5,v1,v1d,ic4,r;
#include<stdlib.h>
#include <LiquidCrystal.h>   //LCD1602a 驱动
LiquidCrystal lcd(8, 7, 6, 5, 4, 3); //(RS,EN,D4,D5,D6,D7)

#include <Wire.h>
#define CONFIG_UNIXTIME 1
#include "ds3231.h"
#include <SD.h>  //sdcard 和vfat的库

#define CHARGE 0
#define TOFULL 1 //charge to full
#define FULLTOZERO 2 //full to zero
#define ZEROTOFULL 3  
uint8_t proc __attribute__ ((section (".noinit")));
uint32_t b1,b2,b3,b4,b5,bf1,bv1 __attribute__ ((section (".noinit")));
uint32_t procxor __attribute__ ((section (".noinit"))); 
uint32_t check() {
  return (b1+proc) ^ b2 ^ b3 ^ b4 ^ b5 ^ bf1 ^ bv1;
}
void calc_check() 
{
  procxor=check();
}
void setproc(uint8_t dat)
{  //存储的校验， 因为proc重启不会清零， 所以要根据校验进行初始化。
  proc=dat;
  calc_check();
}


void sdSave(String dataString) {
  pinMode(10,OUTPUT); //10脚是cs
  File dataFile = SD.open("datalog.csv", FILE_WRITE);
  if(!dataFile) {
    SD.begin(10);
    dataFile = SD.open("datalog.csv", FILE_WRITE);
  }
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
uint8_t getkey()
{ //获取键盘值，返回  1=a摁下，2=b摁下，3=ab都摁下
  uint8_t ret=0;
  boolean sso,miso,scko;  //保存3个原始状态
  sso=digitalRead(10);   
  miso=digitalRead(12);
  scko=digitalRead(13);

  // pinMode(10,OUTPUT); //ss,cs  原本作为sd卡时是输出，不需要修改
  // pinMode(12,INPUT);  //miso  原本作为sd卡时是输入，不需要修改
  pinMode(13,INPUT); //sck    sd卡时是输出， 这里改为输入
  digitalWrite(12,HIGH);   //上拉20k电阻
  digitalWrite(13,HIGH);   //上拉20k电阻
  digitalWrite(10,HIGH);   //输出为1,打开Q3
  delay(1);
  ret|=!digitalRead(12);  //采样按键a 
  ret|=!digitalRead(13)<<1; //采样按键b
  pinMode(13,OUTPUT);    //恢复sd卡口线状态
  digitalWrite(10,sso);
  digitalWrite(12,miso);
  digitalWrite(13,scko);
  if(millis()<4000) return ret;
  lcd.setCursor(1, 0); //设置光标到第一行第14个字符位置
  if(ret&1) lcd.print('*');
  else lcd.print('.');
  lcd.setCursor(5, 0); //设置光标到第一行第14个字符位置
  if(ret&2) lcd.print('*');
  else lcd.print('.');
  return ret;
}

void ad()
{
  uint16_t sv1d,svcc,sif1,sic1,sic2,sic3,sic4,sic5,sv1; 
  const uint16_t ivcc=1.1*1000*10*(24.3+499)/24.3/1024; //2313.3278   0.1mv
  const uint16_t iv1=1.1*1000*10*(97.6+499)/97.6/1024; //65.6638 0.1mv
  const uint16_t ii=1.1*1000*10/(0.33)/1024;//3.255 ma   0.1ma
  float val;
  boolean A5V;
  A5V=digitalRead(A5);
  pinMode(A5,INPUT);
  digitalWrite(A5,LOW);  
  digitalWrite(A4,LOW);
  digitalWrite(11,LOW);  
  delay(1);
  sif1=analogRead(IF1);
  sv1=analogRead(V1);
  sic1=analogRead(IC1);  
  disable();
  delay(1); //!
  sic2=analogRead(IC2);
  sic3=analogRead(IC3);
  sic4=analogRead(IC4);
  sic5=analogRead(IC5);
  svcc=analogRead(VCC);
  sv1d=analogRead(V1);
  digitalWrite(11,HIGH);
  pinMode(A5,OUTPUT);
  if(digitalRead(A5)!=A5V) digitalWrite(A5,A5V);
  oneset();
  val=231.33278*svcc;
  vcc=val;  //mv
  val=6.56638*sv1;
  v1=val;  //mv
  val=6.56638*sv1d;
  v1d=val; //mv
  val=3.255*sif1;
  if1=val; //ma
  val=3.255*sic1;
  ic1=val;  //ma
  val=3.255*sic2;
  ic2=val;  //ma
  val=3.255*sic3;
  ic3=val; //ma
  val=3.255*sic4;
  ic4=val; //ma
  val=3.255*sic5;
  ic5=val;
float x=0;
  if(proc==FULLTOZERO && if1>50) { //放电
   if(sv1<sv1d) {
     x=6.56638*(sv1d-sv1)/sif1/3.255-0.33;
 r=x*1000;  
 }  
} else if(ic1>50) {
    if(v1d<v1) {
      x=6.56638*(sv1-sv1d)/sic1/3.255-0.33;
  r=x*1000;  
  }
    }
    Serial.print("sv1=");
    Serial.print(sv1);
    Serial.print(",sv1d=");
    Serial.print(sv1d);
    
    Serial.print(",sif1=");
    Serial.print(sif1);

    Serial.print(",sic1=");
    Serial.print(sic1);
    Serial.print(",r=");
    Serial.println(r);
}

void oneset()
{
  switch(proc) {
  case CHARGE:
  case TOFULL:
  case ZEROTOFULL:
    charge();
    break;
  case FULLTOZERO:
    Discharge();
    break;
  }
}
boolean i=false;
void setup()
{
  struct ts t;
  uint8_t upchar[8] = {
  0b00100,
  0b01110,
  0b10101,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100
},
downchar[8] = {
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b10101,
  0b01110,
  0b00100
},
wdchar[8] = {
  0b10000,
  0b00000,
  0b01110,
  0b10001,
  0b10000,
  0b10000,
  0b10001,
  0b01110
};
  String hello="Hello,Cfido!V" VER;
  Serial.begin(9600); //串口9600
  Serial.println(hello);
  analogReference(INTERNAL); //使用atmega328的内部1.1V 基准源
  analogRead(A0); //第一次转换不准确， 要用掉
  lcd.begin(16, 2); //lcd初始化  16字符2行
  lcd.createChar(2, upchar);  
  lcd.createChar(1, downchar);
  lcd.createChar(3, wdchar);  
  lcd.clear();
  lcd.print(hello); 
  pinMode(11,OUTPUT);
  digitalWrite(11,HIGH);
  Wire.begin();
  DS3231_init(DS3231_INTCN);
  DS3231_clear_a1f();
  DS3231_clear_a2f();
  DS3231_set_creg(0x4);
  DS3231_get(&t);
  for(int a=0;a<10;a++) {
    delay(100); 
    digitalWrite(11,!digitalRead(11)); //lcd背光煽动10次
  }
  if(check()!=procxor) {
    proc=CHARGE;
    b1=0;
    b2=0;
    b3=0;
    b4=0;
    b5=0;
    bf1=0;
    bv1=0;

    calc_check();  //proc校验不过，就初始化。
  }
  oneset();
}
uint32_t dida=0;
uint8_t keya=0;
void keydown()
{
  switch(getkey()) {
  case 1:
    proc=FULLTOZERO;
    calc_check();
    break;
  case 2:
    proc=CHARGE;
    calc_check();
  } 
}
char dispbuff[18];
float wd;
void disptime()
{
  struct ts t;
  char timestr[18];
  boolean o11=digitalRead(11);
  pinMode(11,OUTPUT);
  digitalWrite(11,HIGH);
  Wire.begin();
  wd=DS3231_get_treg();
  DS3231_get(&t);
  snprintf(dispbuff, 17, "%04d-%02d-%02d %02d:%02d",t.year,
  t.mon, t.mday, t.hour, t.min);
  lcd.setCursor(0, 1);  //设置光标位置到第二行的左边
  lcd.print(dispbuff);   //显示buff到第二行
  return;
}

void ah_add()
{ //10s
  uint32_t ia;
  //if(ib1
}
void loop() 
{ //循环
  String stringVal ; //显示buff
  if(keya!=getkey()) {
    keydown();
    keya=getkey();
  }
  if(dida+2000>millis()) return;
  dida=millis();
  ad();
  Serial.print("wd=");
  Serial.println(wd);
  if(if1>10)
  sprintf(dispbuff,"%01d.%01d %01d.%01d \x01%03d %03d",vcc/10/1000,(vcc/100)%10,v1/1000,(v1/100)%10,if1,r);
  else if(ic1>10)
  sprintf(dispbuff,"%01d.%01d %01d.%02d %03d %d\x03",vcc/10/1000,(vcc/100)%10,v1/1000,(v1/10)%100,ic1,(int)wd);
  else
  sprintf(dispbuff,"%01d.%01d %01d.%02d %d\x03    " VER,vcc/10/1000,(vcc/100)%10,v1/1000,(v1/10)%100,(int)wd);

  Serial.println(dispbuff); //把显示buff送串口
  lcd.setCursor(0, 0); //设置光标到第一行第一个字符位置
  lcd.print(dispbuff);  //显示字符串到第一行
  //开始准备第二行
  stringVal="";
  if(i) {
    snprintf(dispbuff,17,"%03d %03d %03d %03d ",ic2,ic3,ic4,ic5);
    lcd.setCursor(0, 1);  //设置光标位置到第二行的左边
    lcd.print(dispbuff);   //显示buff到第二行
    i=false;
  }//第二行准备完毕
  else {
    i=true;
    disptime();
  } 
  Serial.println(dispbuff); //输出到串口
}

