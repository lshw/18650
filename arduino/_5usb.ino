#define VER "1.3"
//ad管脚定义
#if defined(__AVR_ATmega328P__)
#define IC1 A4 //Vref R0=0.33 充电1 内置基准电压1.1V 采样电阻0.33欧姆  满量程3.33A分辨率3.255208ma
#define IC2 A1 //Vref R0=0.33 充电2
#define IC3 A0 //Vref R0=0.33 充电3
#define IC4 A7 //Vref R0=0.33 充电4
#define IC5 A5 //Vref R0=0.33 充电5
#define IF1 A3 //Vref R0=0.33 放电1
#define VCC A2 //Vref=1.1  1.1*VCC/1024/24.3*(24.3+499); //外接电源电压
#define V1 A6  //Vref=1.1  1.1*V1/1024/97.6*(97.6+499)  //1号电池电压  
#else //atmega32u4
#define IC1 A4 //Vref R0=0.33 充电1 内置基准电压1.1V 采样电阻0.33欧姆  满量程3.33A分辨率3.255208ma
#define IC2 A1 //Vref R0=0.33 充电2
#define IC3 A0 //Vref R0=0.33 充电3
#define IC4 A7 //Vref R0=0.33 充电4
#define IC5 A5 //Vref R0=0.33 充电5
#define IF1 A3 //Vref R0=0.33 放电1
#define VCC A2 //Vref=1.1  1.1*VCC/1024/24.3*(24.3+499); //外接电源电压
#define V1 A6  //Vref=1.1  1.1*V1/1024/97.6*(97.6+499)  //1号电池电压  
#endif

#define CHONG1 1
#define CHONG2 2
#define CHONG3 3
#define CHONG4 4
#define CHONG5 5
#define FANG 6
#define DISPSE_MAX 6
//#define SNSET  11
//EEPROM 地址定义
#define SN_ADDR 10  //10 11 12 13  序列号
#define Wd_al 2 // 2,3,4,5 报警温度
#define Cin_min 6 //6,7,8,9  //温度map()校准参数4个，
#define Cout_min 15 //15 16 17 18
#define Cin_max  19 //19 20 21 22
#define Cout_max 23 //23 24 25 26
#define ADf   27 //27,28,29,30     放电
#define ADc1  31 //31,32,33,34     充电1
#define ADc2  35 //35,36,37,38     充电2
#define ADc3  39 //39,40,41,42     充电3
#define ADc4  43 //43,44,45,46     充电4
#define ADc5  47 //47,48,49,50     充电5
#define ADvcc 51 //51,52,53,54     Vcc
#define ADv1  55 //55,56,57,58     v1
//100 ... 164  //mah   

#include <MsTimer2.h>
#include<stdlib.h>
#include <LiquidCrystal.h>   //LCD1602a 驱动
LiquidCrystal lcd(8,7,6,5,4,3); //(RS,EN,D4,D5,D6,D7)  lcd接这6条腿
#include <EEPROM.h>
#include <Wire.h>
#include "ds3231.h"
#include <SD.h>  //sdcard 和vfat的库

#define CHARGE 0
#define TOFULL 1 //先充电到满
#define FULLTOZERO 2 //然后放电到空 测放电容量
#define ZEROTOFULL 3 //再充到满  测充电容量
uint32_t sn; //序列号
float wd;   //实际温度值
float wd_al; //alert 温度值
float swd;  //adc
struct ts t; //realtime
float advcc,adv1,adc[6],wdin_min,wdin_max,wdout_min,wdout_max;   //各种校准数值
uint16_t ic[6],vcc,ic5,v1,v1d,r;  //各种测试值  
uint16_t sv1d,svcc,sic[6],sv1; 
char dispbuff[18];  //显示缓冲区

uint8_t setCount __attribute__ ((section (".noinit"))); //复位计数，用于进设置，校准等功能模块
uint8_t proc __attribute__ ((section (".noinit")));   //当前进程
uint32_t b[6] ;  //累计值  b[0]=放电累计  b[1] 充电1累计...
uint8_t procxor __attribute__ ((section (".noinit")));
uint8_t check() {
  return proc^'L';
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

//从eeprom读取一个浮点数
float eeprom_float_read(uint16_t address) {
  float fl;
  uint8_t * fli = (uint8_t *) &fl;
  fli[0]=EEPROM.read(address);
  fli[1]=EEPROM.read(address+1);
  fli[2]=EEPROM.read(address+2);
  fli[3]=EEPROM.read(address+3);
  return fl;
}

//写一个浮点数到eeprom
void eeprom_float_write(uint16_t address,float val) {
  float fl;
  uint8_t * fli = (uint8_t *) &fl;
  if(eeprom_float_read(address)==val) return; //相同就不写
  fl=val;
  EEPROM.write(address,fli[0]);
  EEPROM.write(address+1,fli[1]);
  EEPROM.write(address+2,fli[2]);
  EEPROM.write(address+3,fli[3]);
  return ;
}
void sdSave(char * dataString) {
  //存字符串到sdcard的datalog.csv
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
  else {
    Serial.println("sdcard error!");
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
  return ret;
}

void ad()
{
  float val;  //存放中间结果
  boolean A5V;
  A5V=digitalRead(A5); //保存A5的值
  pinMode(A5,INPUT);
  digitalWrite(A5,LOW);  
  digitalWrite(A4,LOW);
  digitalWrite(11,LOW);  
  delay(1);
  sic[0]=analogRead(IF1);
  sv1=analogRead(V1);
  sic[1]=analogRead(IC1);  
  disable();
  delay(1); //!
  sic[2]=analogRead(IC2);
  sic[3]=analogRead(IC3);
  sic[4]=analogRead(IC4);
  sic[5]=analogRead(IC5);
  svcc=analogRead(VCC);
  sv1d=analogRead(V1);
  digitalWrite(11,HIGH);
  pinMode(A5,OUTPUT);
  if(digitalRead(A5)!=A5V) digitalWrite(A5,A5V);
  oneset();
  val=advcc*svcc;
  vcc=val;  //0.1mv
  val=adv1*sv1;
  v1=val;  //mv
  val=adv1*sv1d;
  v1d=val; //mv
  for(uint8_t i1=0;i1<6;i1++){
  val=adc[i1]*sic[i1];
  ic[i1]=val; //ma
  }
  float x=0;
  if(proc==FULLTOZERO && ic[0]>50) { //放电
    if(sv1<sv1d) {
      x=adv1*(sv1d-sv1)/sic[0]/adc[0]-0.33; //0.33?
      r=x*1000;  
    }  
  } 
  else if(ic[1]>50) {
    if(v1d<v1) {
      x=adv1*(sv1-sv1d)/sic[1]/adc[1]-0.33; //0.33?
      r=x*1000;  
    }
  }
}

void oneset()
{ //根据当前进程，设置充电/放电状态
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
uint8_t bz;
void setwd_al(){ //设置报警温度
  uint8_t key;
  disptime();
  lcd.clear();
  lcd.print("set alert \x03");
  lcd.setCursor(0,1);
  lcd.print(wd_al);
  lcd.print("\x03 alert");
  waitKeyUp();
  setCount--;
  for(;;){
    waitKeyDown();
    key=getkey();
    if(key==1) {
      wd_al += 0.25;
    }
    else if(key==2) {
      wd_al -= 0.25;
    }
    waitKeyUp();
    lcd.setCursor(0,1);
    lcd.print(wd_al);
    eeprom_float_write(Wd_al, wd_al);

  }
}
void setwd(){
  uint8_t key;
  disptime();
  lcd.clear();
  lcd.print("set ");
  lcd.print(swd);
  lcd.print("\x03");
  lcd.setCursor(0,1);
  lcd.print(wd);
  lcd.print("\x03");
  waitKeyUp();
  setCount--;

  if(swd <6.0) {
    lcd.print(" set zero ");
  }
  else{
    lcd.print(" set High ");
  }
  for(;;) {
    waitKeyDown();
    key=getkey();
    if(key==1) {
      wd += 0.25;
    }
    else if(key==2) {
      wd -= 0.25;
    }
    waitKeyUp();
    lcd.setCursor(0,1);
    lcd.print(wd);
    if(swd<6.0) {
      eeprom_float_write(Cin_min, swd);
      eeprom_float_write(Cout_min,wd);
    }
    else {
      eeprom_float_write(Cin_max,swd);
      eeprom_float_write(Cout_max,wd);
    }
    Serial.print("Cin_min=");
    Serial.print(eeprom_float_read(Cin_min));
    Serial.print(",Cout_min=");
    Serial.print(eeprom_float_read(Cout_min));
    Serial.print(",Cin_max=");
    Serial.print(eeprom_float_read(Cin_max));
    Serial.print(",Cout_max=");
    Serial.println(eeprom_float_read(Cout_max));
  }
}
void setJz1() //放电1校准
{
  lcd.print("Calibration F1");
  waitKeyUp();
  setCount--;
  Discharge();
  ad();
  Calibration(IF1);
}
void setJz2()
{
  lcd.print("Calibration V1");
  waitKeyUp();
  setCount--;
  ad();
  Calibration(V1);
}

void setJz() {  //校准充电2-5 ,vcc,v1
  lcd.print("Calibration C1-5");
  waitKeyUp();
  setCount--;
  ad();

  for(;;) {
    if(sic[1]>50) {
      Calibration(IC1);
    }
    else if(sic[2]>50){
      Calibration(IC2);
    }
    else if(sic[3]>50){
      Calibration(IC3);
    }
    else if(sic[4]>50){
      Calibration(IC4);
    }
    else if(sic[5]>50){
      Calibration(IC5);
    }
    else {
      Calibration(VCC);
    }
  }
}
uint16_t a2i(uint8_t offs,uint8_t count) {
  uint16_t val=0;
  for(;count>0;count--) {
    val*=10;
    val+=dispbuff[offs]&0xf;
    offs++;
  }
  return val;
}
uint16_t set_mv(char * name,uint16_t  val) {
  char ch;
  ch='v';
  if(val<4000) val=4000;
  if(val>7000) val=7000;
  sprintf(dispbuff,"%s=%dmv        ",name, val);
  lcd.print(dispbuff);
  modidisp("hhhh4000hhhhhhhh","hhhh7999hhhhhhhh");
  val=a2i(4,4);
  return val;
}
uint16_t set_ma(char * name,uint16_t val) {
  char ch;
  ch='v';
  if(val<100) val=100;
  if(val>700) val=700;
  sprintf(dispbuff,"%s=%dma        ",name,val);
  lcd.print(dispbuff);
  modidisp("hhh100hhhhhhhhhh","hhh799hhhhhhhhh"); 
  val=a2i(3,3);
  return val;
}
void Calibration(uint8_t adpin) {
  float val;
  bz=0;
  lcd.setCursor(0,1);
  for(;;){
    switch(adpin) {
    case VCC:
      vcc=set_mv("Vcc",vcc);
      val=vcc;
      advcc=val/svcc;
      eeprom_float_write(ADvcc,advcc);
      break;
    case V1:
      v1=set_mv("V1 ",v1);
      val=v1;
      adv1=val/sv1;
      eeprom_float_write(ADv1,adv1);
      break;
    case IF1:
      ic[0]=set_ma("F1",ic[0]);
      val=ic[0];
      adc[0]=val/sic[0];
      eeprom_float_write(ADf,adc[0]);
      break;
    case IC1: 
      ic[1]=set_ma("C1",ic[1]);
      val=ic[1];
      adc[1]=val/sic[1];
      eeprom_float_write(ADc1,adc[1]);
      break;
    case IC2:
      ic[2]=set_ma("C2",ic[2]);
      val=ic[2];
      adc[2]=val/sic[2];
      eeprom_float_write(ADc2,adc[2]);
      break;
    case IC3:
      ic[3]=set_ma("C3",ic[3]);
      val=ic[3];
      adc[3]=val/sic[3];
      eeprom_float_write(ADc3,adc[3]);
      break;
    case IC4:
      ic[4]=set_ma("C4",ic[4]);
      val=ic[4];
      adc[4]=val/sic[4];
      eeprom_float_write(ADc4,adc[4]);
      break;
    case IC5:
      ic[5]=set_ma("C5",ic[5]);
      val=ic[5];
      adc[5]=val/sic[5];
      eeprom_float_write(ADc5,adc[5]);
      break;
    }
  }
}
void modidisp(char * mins,char * maxs){
  uint8_t key;
  for(uint8_t n=0;n<16;n++){
    if(mins[bz]=='h'){
      bz++;
      if(bz==16) bz=0;  
    }
    else 
      break;
  }

  lcd.setCursor(bz,1);
  lcd.blink();
  waitKeyDown();
  key=getkey();
  Serial.print("key=");
  Serial.println(key);
  waitKeyUp();
  lcd.noBlink();
  if(key==2) {
    bz++;
    if(bz==16) bz=0;
  }
  else if(key==1){
    if(dispbuff[bz]<=maxs[bz] & dispbuff[bz] >=mins[bz]) {
      dispbuff[bz]++;
      if(dispbuff[bz]>maxs[bz]) dispbuff[bz]=mins[bz];
    }
    Serial.print("b=");
    Serial.println(dispbuff);
  }
  if(dispbuff[bz]<mins[bz]) dispbuff[bz]=mins[bz];
  if(dispbuff[bz]>maxs[bz]) dispbuff[bz]=maxs[bz];

  lcd.setCursor(0, 1);  
  lcd.print(dispbuff);
}
void waitKeyUp()
{
  for(;;)  if(!getkey()) break;
}   
void waitKeyDown()
{
  for(;;)  if(getkey()) break;
}   
void setTime(){
  bz=0;
  lcd.print("setTime ");
  lcd.print(setCount);
  waitKeyUp();
  for(;;) {
    lcd.noBlink();
    disptime();
    lcd.setCursor(bz,1);
    lcd.blink();
    modidisp("hh10h00h00h00h00","hh49h19h39h29h59");
    //2015-07-21 00:00   2046-12-31 23:59

    t.sec = 00;
    t.min = a2i(14,2);//(dispbuff[14]&0xf)*10+dispbuff[15]&0xf;//inp2toi(dispbuff, 14);
    t.hour = a2i(11,2);//(dispbuff[11]&0xf)*10+dispbuff[12]&0xf;//inp2toi(dispbuff, 11);
    t.wday=1;
    t.mday = a2i(8,2);//(dispbuff[8]&0xf)*10+dispbuff[9]&0xf;//inp2toi(dispbuff, 8);
    t.mon = a2i(5,2);//inp2toi(dispbuff, 5);
    t.year = 2000 + a2i(2,2);//inp2toi(dispbuff, 2);
    if(t.year>=2048) t.year=2015;
    if(t.mday==0) t.mday=1;
    if(t.mday>31)  t.mday=30;
    if(t.hour>59) t.hour=50;
    if(t.mon>12 | t.mon==0) t.mon=10;
    if(t.mon==2 && t.mday>29) t.mday=20;
    if(t.mon==4 | t.mon==6 |t.mon==9 | t.mon==11) 
      if(t.mday>30) t.mday=30;
    Wire.begin();
    DS3231_init(DS3231_INTCN);
    DS3231_set(t);    
  }
}
void save_eeprom(){
  //存字符串到sdcard的datalog.csv
  lcd.clear();
  lcd.print("EEPROM to file");
  waitKeyUp();
  setCount--;
  pinMode(10,OUTPUT); //10脚是cs
  lcd.setCursor(0,1);
  //sprintf(dispbuff,"eeprom_%d.bin",sn);
  //lcd.print(dispbuff);
  File dataFile = SD.open("eeprom.bin", FILE_WRITE);
  if(!dataFile) {
    SD.begin(10);
    dataFile = SD.open("eeprom.bin", FILE_WRITE);
  }
  if (dataFile) {
    for( uint16_t n=0;n<512;n++) {
      dataFile.write(EEPROM.read(n));
    }
    dataFile.close();
    lcd.setCursor(14,1);
    lcd.print("OK");
  }
  else {
    lcd.setCursor(13,1);
    lcd.print("ERR");
  }
  for(;;) ;
}
boolean i=false;
void calc_sum() {
  for(uint8_t i1=0;i1<6;i1++) 
  if(ic[i1]>40) b[i1]+=ic[i1];
}

void setup()
{
  uint8_t upchar[8] = {  /*上箭头*/
    0b00100,
    0b01110,
    0b10101,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00100
  }
  ,
  downchar[8] = { /*下箭头*/
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b10101,
    0b01110,
    0b00100
  }
  ,
  wdchar[8] = { /*摄氏度*/
    0b10000,
    0b00000,
    0b01110,
    0b10001,
    0b10000,
    0b10000,
    0b10001,
    0b01110
  }
  ,
  oumchar[8] = {  /*欧姆*/
    0b11011,
    0b10101,
    0b10101,
    0b00000,
    0b11111,
    0b10001,
    0b01010,
    0b11011
  }
  ,
  machar[8] = {  /* ma */
    0b11011,
    0b10101,
    0b10101,
    0b10001,
    0b00100,
    0b01010,
    0b01110,
    0b10001
  };

  //载入序列号
  sn=EEPROM.read(SN_ADDR + 3) << 24;
  sn+=EEPROM.read(SN_ADDR + 2) << 16;
  sn+=EEPROM.read(SN_ADDR + 1) << 8;
  sn+=EEPROM.read(SN_ADDR);

  //载入校准值
  adc[0]=eeprom_float_read(ADf);  //放电ad
  adc[1]=eeprom_float_read(ADc1); //充电1ad
  adc[2]=eeprom_float_read(ADc2);  //充电2ad
  adc[3]=eeprom_float_read(ADc3);  //充电3ad
  adc[4]=eeprom_float_read(ADc4);  //充电4ad
  adc[5]=eeprom_float_read(ADc5);  //充电5ad
  advcc=eeprom_float_read(ADvcc);
  adv1=eeprom_float_read(ADv1);
  wd_al=eeprom_float_read(Wd_al);   //alert 温度
  wdin_min=eeprom_float_read(Cin_min);   //0度时对应的AD值
  wdin_max=eeprom_float_read(Cin_max);     //Cout_max温度对应的 AD值
  wdout_min=eeprom_float_read(Cout_min);       //温度线性校准，一个点在0度， 另一个点取某气温。
  wdout_max=eeprom_float_read(Cout_max);   //某温度
  char hello[]="Cfido!V" VER " ";
  Serial.begin(9600); //串口9600
  Serial.println(hello);
  //sdSave("hhhhhh");
  analogReference(INTERNAL); //使用atmega328的内部1.1V 基准源
  analogRead(A0); //第一次转换不准确， 要用掉
  lcd.begin(16, 2); //lcd初始化  16字符2行
  lcd.createChar(2, upchar);   //^
  lcd.createChar(1, downchar); //V
  lcd.createChar(3, wdchar);  //sheshidu
  lcd.createChar(4, oumchar);   //om
  lcd.createChar(5, machar);  //ma  
  lcd.clear();
  lcd.print(hello); 
  lcd.print("SN=");
  lcd.print(sn);
  pinMode(11,OUTPUT);
  digitalWrite(11,HIGH);
  Wire.begin();
  DS3231_init(DS3231_INTCN);
  DS3231_clear_a1f();
  DS3231_clear_a2f();
  DS3231_set_creg(0x4);
  if(setCount>8) setCount=0;
  if(getkey()==1) setCount++;
  if((DS3231_get_sreg() & 0x80) | getkey()==1) {
    DS3231_set_sreg(0);
    lcd.clear();
    switch(setCount) {
    case 3:
      setwd();
      break;
    case 4:
      setwd_al(); //设置报警温度
      break;
    case 5:
      setJz();  //校准充电1-5,Vcc
      break;
    case 6:
      setJz1();  //放电1校准
      break;
    case 7:
      setJz2();  //校准V1
      break;
    default:
      setTime();
    }
  }
  setCount=0;
  disptime();
  for(int a=0;a<20;a++) {
    delay(100); 
    digitalWrite(11,!digitalRead(11)); //lcd背光煽动20次
  }
  if(check()!=procxor) {
    proc=CHARGE;
    calc_check();  //proc校验不过，就初始化。
  }
  oneset();
  MsTimer2::set(1000, calc_sum); // 1s period
  MsTimer2::start();
}
uint32_t dida=0,dispHoldTime=millis();
uint8_t keya=0;
int8_t dispse=0;
void keydown()
{
  switch(getkey()) {
  case 1:
  dida=millis();
    waitKeyUp();
 if(proc==CHARGE & (millis()-dida) > 3000){
 setproc(TOFULL);
 return;
 }
     dispse++;
     dispHoldTime=millis()+10000;  
    break;
  case 2:
  dida=millis();
  waitKeyUp();
  if(proc!=CHARGE & (millis()-dida) >3000) {
  setproc(CHARGE);
  return;
  }
  dispse--;
dispHoldTime=millis()+10000;  
  break;
  default:
  return;
} 
if(dispse<0) dispse=DISPSE_MAX;
if(dispse>DISPSE_MAX) dispse=0;
}

void disptime()
{
  pinMode(11,OUTPUT);
  digitalWrite(11,HIGH);
  Wire.begin();
  DS3231_get(&t);
  swd=DS3231_get_treg();
  //温度校准
  wd=(swd - wdin_min) * (wdout_max - wdout_min)/(wdin_max - wdin_min) + wdout_min;
  wd=wd*4;
  wd=(int)wd;
  wd=wd/4;
  sprintf(dispbuff,  "%04d-%02d-%02d %02d:%02d",t.year,
  t.mon, t.mday, t.hour, t.min);
  lcd.setCursor(0, 1);  //设置光标位置到第二行的左边
  lcd.print(dispbuff);   //显示buff到第二行

  return;
}

void displog(char * msg,int8_t sel){
lcd.clear();
lcd.print(msg);
lcd.print(":");
if(ic[sel]>0){
lcd.print("\x02");
lcd.print(b[sel]/3600);
lcd.print("mah");
}
lcd.setCursor(0,1);
for(uint8_t i=0;i<16;i++) lcd.write(EEPROM.read(100+16*sel+i)); //日期 [100] 放电 ， [116] 充1 ，[132] 充2，[148] 充3 .... 

}

uint16_t eeprom_int16_read(uint16_t addr){
return((uint16_t)EEPROM.read(addr)+(uint16_t)(EEPROM.read(addr+1)<<8));
}
void eeprom_int16_write(uint16_t addr,uint16_t dat) {
if(eeprom_int16_read(addr)==dat) return;
EEPROM.write(addr,dat&0xff);
EEPROM.write(addr+1,(dat>>8)&0xff);
}
//[100]-[109] 2015-07-01 [110]-[113] float mah, [114]-[116] uint16_t ma,
boolean cc[6]={0,0,0,0,0,0};
void disp() {
  if(dispse > 0 & dispHoldTime>millis()) { 
switch(dispse){
case FANG:
displog("F1",FANG);
break;
case CHONG1:
displog("C1",CHONG1);
break;
case CHONG2:
displog("C2",CHONG2);
break;
case CHONG3:
displog("C3",CHONG3);
break;
case CHONG4:
displog("C4",CHONG4);
break;
case CHONG5:
displog("C5",CHONG5);
break;
default:
break;
}
lcd.setCursor(15,0);
lcd.print((dispHoldTime-millis())/1000);
return;
  }
  if(ic[0]>10)
    sprintf(dispbuff,"%01d.%01d %01d.%01d %03d\x01%03d\x04",vcc/1000,(vcc/100)%10,v1/1000,(v1/100)%10,ic[0],r);
  else if(ic[1]>10)
    sprintf(dispbuff,"%01d.%01d %01d.%02d %03d %d\x03",vcc/1000,(vcc/100)%10,v1/1000,(v1/10)%100,ic[1],(int)wd);
  else{
    sprintf(dispbuff,"%01d.%01d %01d.%02d      " VER,vcc/1000,(vcc/100)%10,v1/1000,(v1/10)%100);
  }
  Serial.println(dispbuff); //把显示buff送串口
  lcd.setCursor(0, 0); //设置光标到第一行第一个字符位置
  lcd.print(dispbuff);  //显示字符串到第一行
  if(ic[0]<=10 && ic[1]<=10) {
    lcd.setCursor(10,0);
    lcd.print(wd);
    lcd.print("\x03");
  }
  //开始准备第二行
  if(i) {
    if( ic[2]!=0 | ic[3]!=0 | ic[4]!=0 | ic[5] != 0 )  {
    sprintf(dispbuff,"%03d %03d %03d %03d ",ic[2],ic[3],ic[4],ic[5]);
    lcd.setCursor(0, 1);  //设置光标位置到第二行的左边
    lcd.print(dispbuff);   //显示buff到第二行
    }else{
    lcd.setCursor(13,1);
    lcd.print(" ");
    }
    i=false;
  }//第二行准备完毕
  else {
    i=true;
    disptime();
  }
  Serial.println(dispbuff); //输出到串口
}
boolean have100ma[6]={false,false,false,false,false,false},have0ma[6]={false,false,false,false,false,false};
void proc_select() {
switch(proc) {
case TOFULL:
if(ic[1]>100 & ic[1] <150) have100ma[0]=true;
if(ic[1]==0 & have100ma[1]==true) {
have100ma[1]=false;
setproc(FULLTOZERO);
b[0]=0;
}
break;
case FULLTOZERO:
if(v1>3600) {  //3.6V终止放电电压
b[1]=0;
have0ma[1]=false;
setproc(ZEROTOFULL);
}
break;
case ZEROTOFULL:
if(ic[1]>100 & ic[1] <150) have100ma[1]=true;
if(ic[1]==0 & have100ma[1]==true) {
have100ma[1]=false;
setproc(CHARGE);
}
break;
}
}
void save(uint8_t sel)
{
sprintf(dispbuff,"%04d-%02d-%02d %04d",t.year,t.mon,t.mday,b[sel]/3600);
for(uint8_t i1=0;i1<15;i++) EEPROM.write(100+sel*16+i1,dispbuff[i1]);
}
void fd() {
for(uint8_t i1=1;i1<6;i1++) {  //只处理放电 1-5
if(ic[i1]>100 & ic[i1]<150) have100ma[i1]=true; //经过了100ma这一道，才会在0ma时保存结果， 像电池突然拿下来，再放回去，不影响继续测试
if(ic[i1]==0) { //结束充电
  have0ma[i1]=true;  //到了0ma
if(have100ma[i1]==true) {
save(i1);  //保存结果
have100ma[i1]=false; //下次不再保存。
}
}

if(ic[i1]>400 & have0ma[i1]==true) { //到过0ma才会是下次测试的开始
b[i1]=0;
have0ma[i1]=false;
}

}
}

void loop()
{ //循环
if(wd>wd_al) digitalWrite(11,millis()/100%2);
proc_select();
fd();
if(keya!=getkey()) {
    keya=getkey();
    keydown();
    disp();
  }
  if(dida+1000>millis()) return; 
  dida=millis();
  ad();  
  disp();
}

