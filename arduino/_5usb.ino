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

#include <Wire.h>
#include "ds3231.h"

#include <SD.h>  //sdcard 和vfat的库


#define CHARGE 0
#define TOFULL 1 //charge to full
#define FULLTOZERO 2 //full to zero
#define ZEROTOFULL 3  
uint8_t proc __attribute__ ((section (".noinit"))); 
uint8_t procxor __attribute__ ((section (".noinit"))); 

void setproc(uint8_t dat)
{  //存储的校验， 因为proc重启不会清零， 所以要根据校验进行初始化。
  proc=dat;
  procxor=dat^'L';
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
  return ret;
}
uint16_t getval(int VIN)   //读取电流电压值  电流为ma ，电压为mV数/10
{
  uint32_t val; //每个数字都乘以100倍,是为了保留足够的小数位数
  boolean i11,A5V;
i11=digitalRead(11); 
A5V=digitalRead(A5);
  pinMode(VIN,INPUT);
  digitalWrite(VIN,LOW);
  delay(1);  //这个要测试一下，是否可以缩短。
  if(VIN==A4) {
  digitalWrite(11,LOW);  
  delay(1);
  }
  switch(VIN) {
  case VCC: //因为选的都是0.1%精度的电阻，所以不需要校准，就可以根据计算保证精度 
    val=1.1*1000*100*(24.3+499)/24.3/1024;  
    break;
  case V1:   //10mv
    val=1.1*1000*100*(97.6+499)/97.6/1024;                       
    break;
  default:  //ma
    val=1.1*1000*100/(0.33)/1024;    //1000->换算成ma, 1024->10位AD, 0.33->取样电阻
  }
  val=val*analogRead(VIN)/100;
  digitalWrite(11,HIGH);
  if(VIN==VCC || VIN==V1) val=val/10;
  pinMode(A5,OUTPUT);
  if(digitalRead(A5)!=A5V) digitalWrite(A5,A5V);
  return(val); 
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
    String hello="Hello,Cfido!";
  Serial.begin(9600); //串口9600
  Serial.println(hello);
  analogReference(INTERNAL); //使用atmega328的内部1.1V 基准源
  analogRead(A0); //第一次转换不准确， 要用掉
  lcd.begin(16, 2); //lcd初始化  16字符2行
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
  if(proc^'L'!=procxor) setproc(CHARGE);  //proc校验不过，就初始化。
  oneset();
}
uint32_t dida=0;
uint8_t keya=0;
void keydown()
{
  Serial.println(getkey(),BIN);
}
void loga(char ch)
{
   lcd.setCursor(15, 0); //设置光标到第一行第一个字符位置
 lcd.print(ch);  //显示字符串到第一行
  
}
void disptime()
{
  struct ts t;
  char timestr[18];
 boolean o11=digitalRead(11);
pinMode(11,OUTPUT);
digitalWrite(11,HIGH);
    Wire.begin();
    DS3231_get(&t);
    snprintf(timestr, 17, "%04d-%02d-%02d %02d:%02d",t.year,
             t.mon, t.mday, t.hour, t.min);
  lcd.setCursor(0, 1);  //设置光标位置到第二行的左边
  lcd.print(timestr);   //显示buff到第二行
return;

}
void loop() 
{ //循环
  String stringVal ; //显示buff
  uint16_t va; //存放模拟量的值
  if(keya!=getkey()) {
  keydown();
  keya=getkey();
  }
  if(dida+2000>millis()) return;
  
  dida=millis();
  if(proc!=ZEROTOFULL && proc !=FULLTOZERO) {
  disable(); // 关闭1号电池的充放电，
  }

  va=getval(VCC); //测电源电压
  stringVal=String(va/100)+"."+String(va%100/10);  //换算成字符串 
  va=getval(V1); //测一号电池的电压， 关闭充放电的情况下测试
  oneset(); //1号电池测完，测完打开到充电模式
  stringVal +="V "+String(va/100)+"."+getmv(va%100)+"V "; //换算成字符串
  va=getval(IF1); //放电电流
  if(va>0) 
    stringVal +="-"+getma(va)+" "; //如果不是0,则作为负值放入显示buff

  va=getval(IC1); //测充电电流
  if(va>0)
    stringVal += getma(va)+"ma"; //放入显示buff
   Serial.println(stringVal); //把显示buff送串口
  lcd.setCursor(0, 0); //设置光标到第一行第一个字符位置
  lcd.print(stringVal+" ");  //显示字符串到第一行
  //开始准备第二行
 stringVal="";
 if(i) {
   stringVal = getma(getval(IC2))+" "; //把2号电流放入buff，3位数字加一个空格 
  stringVal+=getma(getval(IC3))+" "; //把3号电流放入buff，3位数字加一个空格
  stringVal+=getma(getval(IC4))+" "; //把4号电流放入buff，3位数字加一个空格
  stringVal+=getma(getval(IC5))+" "; //把5号电流放入buff，3位数字加一个空格
 lcd.setCursor(0, 1);  //设置光标位置到第二行的左边
  lcd.print(stringVal);   //显示buff到第二行

i=false;
 }//第二行准备完毕
else {
  i=true;
disptime();
} 
  Serial.print(stringVal+"\r\n"); //输出到串口
 }
