#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "FS.h"
#include <LITTLEFS.h>
#include <IOXhop_FirebaseESP32.h>
#include <LiquidCrystal_I2C.h> 
#include "WebPage.h"
#include <WebServer.h>

#include <Adafruit_MPU6050.h>
#include <Adafruit_BMP085.h>
#include "MAX30105.h"           //sparkfun MAX3010X library
#include "heartRate.h"

#define USEFIFO    
#define FORMAT_LITTLEFS_IF_FAILED true

#define DEBUG
#define FORMAT_LITTLEFS_IF_FAILED true
#define SERIAL_SPEED 115200
#define LOOP_DELAY 1000

MAX30105 particleSensor;
LiquidCrystal_I2C lcd(0x27, 16, 2);
const byte onboard_led = 2;

//=================================================================================
//Firebase
#define WIFI_SSID "your_wifi"
#define WIFI_PASSWORD "your_password"
#define FIREBASE_HOST "link_to_host.firebase.com"
#define FIREBASE_AUTH "key"
long fireDelay;
bool switchFirebase;
//=================================================================================
//LCD
uint8_t heart[8] = {0x0, 0xa, 0x1f, 0x1f, 0xe, 0x4, 0x0};
//=================================================================================
//MPU
const int MPU_addr=0x68;  // I2C address of the MPU-6050
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
float ax=0, ay=0, az=0, gx=0, gy=0, gz=0;
boolean fall = false; //stores if a fall has occurred
boolean trigger1=false; //stores if first trigger (lower threshold) has occurred
boolean trigger2=false; //stores if second trigger (upper threshold) has occurred
boolean trigger3=false; //stores if third trigger (orientation change) has occurred
byte trigger1count=0; //stores the counts past since trigger 1 was set true
byte trigger2count=0; //stores the counts past since trigger 2 was set true
byte trigger3count=0; //stores the counts past since trigger 3 was set true
int angleChange=0;
//=================================================================================
//BMP
const int BMP=0x77;
Adafruit_BMP085 bmp180;
//=================================================================================
//MAX
const byte RATE_SIZE = 16; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
float beatsPerMinute;
int beatAvg;
int beatPast;

//=================================================================================
//=================================================================================
//=================================================================================

WebServer server(80);
File file;

String ap_ssid = "GrannyWatch";
String ap_password = "12345678";
String sta_ssid, sta_password;


//=================================================================================
//=================================================================================
//=================================================================================

void setup() 
{
  #ifdef DEBUG
    Serial.begin(SERIAL_SPEED);
  #endif
  set_pins();
  LITTLEFS.begin(FORMAT_LITTLEFS_IF_FAILED);
  init_device();
 
//=================================================================================
//BMP
  if (!bmp180.begin()){
    Serial.println("Erro!");
    while (1) {}}
  else {
    Serial.println("BMP180 ok");
  }
//=================================================================================
//MPU    
 Wire.begin();
 Wire.beginTransmission(MPU_addr);
 Wire.write(0x6B);  // PWR_MGMT_1 register
 Wire.write(0);     // set to zero (wakes up the MPU-6050)
 Wire.endTransmission(true);
//=================================================================================
//MAX
  if(!particleSensor.begin(Wire, I2C_SPEED_FAST)){
    Serial.println("MAX30102 was not found.");
    while (1);
    }
  else{ 
    Serial.println("MAX30102 ok");
    }
  byte ledBrightness = 255; //Options: 0=Off to 255=50mA
  byte sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  //Options: 1 = IR only, 2 = Red + IR on MH-ET LIVE MAX30102 board
  int sampleRate = 1600; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
//=================================================================================
//LCD    
  lcd.begin();
  lcd.createChar(0, heart);
  lcd.home();
  lcd.setCursor(0, 0);
  lcd.print("GRANNYWATCH");
}

//=================================================================================
//=================================================================================
//=================================================================================
 
void loop(){
//=================================================================================
//BMP
   
   Serial.print("Temperatura : ");
   if ( bmp180.readTemperature() < 10){
     Serial.print(bmp180.readTemperature());
     Serial.println(" C");
   }
   else{ 
     Serial.print(bmp180.readTemperature(),1);
     Serial.println(" C");
   }
     Serial.print("Pressao : ");
     Serial.print(bmp180.readPressure());  
     
     Serial.println(" Pa");

//=================================================================================
//MPU

 Wire.beginTransmission(MPU_addr);
 Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
 Wire.endTransmission(false);
 Wire.requestFrom(MPU_addr,14,1);  // request a total of 14 registers
 AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
 AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
 AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
 Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
 GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
 GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
 GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

 Serial.println(Tmp);
 
 ax = (AcX-2050)/16384.00;
 ay = (AcY-77)/16384.00;
 az = (AcZ-1947)/16384.00;
 gx = (GyX+270)/131.07;
 gy = (GyY-351)/131.07;
 gz = (GyZ+136)/131.07;
 
 float Raw_Amp = pow(pow(ax,2)+pow(ay,2)+pow(az,2),0.5);  // calculating Amplitute vactor for 3 axis
 int Amp = Raw_Amp * 10;  // Mulitiplied by 10 bcz values are between 0 to 1
 //Serial.print("Aceleração: ");
 //Serial.print(Amp);
 //Serial.println(" g");
 
 if (Amp<=2 && trigger2==false){  //if AM breaks lower threshold (0.4g)
   trigger1=true;
   Serial.println("TRIGGER 1 ACTIVATED");
   }
 if (trigger1==true){
   trigger1count++;
   if (Amp>=12){ //if AM breaks upper threshold (3g)
     trigger2=true;
     Serial.println("TRIGGER 2 ACTIVATED");
     trigger1=false; trigger1count=0;
     }
 }
 if (trigger2==true){
   trigger2count++;
   angleChange = pow(pow(gx,2)+pow(gy,2)+pow(gz,2),0.5); Serial.println(angleChange);
   if (angleChange>=30 && angleChange<=400){ //if orientation changes by between 80-100 degrees
     trigger3=true; trigger2=false; trigger2count=0;
     Serial.println(angleChange);
     Serial.println("TRIGGER 3 ACTIVATED");
       }
   }
 if (trigger3==true){
    trigger3count++;
    if (trigger3count>=10){ 
       angleChange = pow(pow(gx,2)+pow(gy,2)+pow(gz,2),0.5);
       //delay(10);
       Serial.println(angleChange); 
       if ((angleChange>=0) && (angleChange<=10)){ //if orientation changes remains between 0-10 degrees
           fall=true; trigger3=false; trigger3count=0;
           Serial.println(angleChange);
             }
       else{ //user regained normal orientation
          trigger3=false; trigger3count=0;
          Serial.println("TRIGGER 3 DEACTIVATED");
       }
     }
  }
 if (fall==true){ //in event of a fall detection
   Serial.println("FALL DETECTED");
   fall=false;
   }
   
 if (trigger2count>=6){ //allow 0.5s for orientation change
   trigger2=false; trigger2count=0;
   Serial.println("TRIGGER 2 DECACTIVATED");
   }
 if (trigger1count>=6){ //allow 0.5s for AM to break upper threshold
   trigger1=false; trigger1count=0;
   Serial.println("TRIGGER 1 DECACTIVATED");
   }

//=================================================================================
//=================================================================================
//MAX

  long irValue = particleSensor.getIR();
  if (irValue < 30000){
    Serial.print(".");
  }
  else{
    
  if (checkForBeat(irValue) == true){
    #ifdef DEBUG
    Serial.print("==========Beat==========");
    Serial.println();
    #endif
    
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte xs = 0 ; xs < RATE_SIZE ; xs++)
        beatAvg += rates[xs];
      beatAvg /= RATE_SIZE;
    }
  }

//===============================
  //if(beatPast != beatAvg){
    Serial.print("IR=");
    Serial.print(irValue);
    Serial.print(", Avg BPM=");
    Serial.print(beatAvg);
    Serial.println();
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("GRANNYWATCH");
    lcd.setCursor(0,1);
    lcd.write(0);
    lcd.print(beatAvg);


    //Firebase.setInt("/ESP1/batimentos/teste", beatAvg);
    
   // }
//===============================
  beatPast = beatAvg;
  }
//=================================================================================




}
//=================================================================================
//=================================================================================
//=================================================================================
//=================================================================================
//=================================================================================

void set_pins(){
  pinMode(onboard_led, OUTPUT);
  digitalWrite(onboard_led, LOW);
}

void start_ap_sta(){
    WiFi.mode(WIFI_OFF);
    delay(10);
    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.softAP(ap_ssid.c_str(), ap_password.c_str());
    
    #ifdef DEBUG
      Serial.println("Ponto de acesso iniciado!");
    #endif
}

bool start_sta(){
    WiFi.mode(WIFI_OFF);
    delay(10);
    WiFi.mode(WIFI_MODE_STA);
    WiFi.begin(sta_ssid.c_str(), sta_password.c_str());
    #ifdef DEBUG
      Serial.print("Conectando-se a rede: ");
      Serial.println(sta_ssid);
    #endif
    unsigned now = millis();  
    bool state = false;
    while((WiFi.status() != WL_CONNECTED) && (millis() - now <= 10000)){
        #ifdef DEBUG
          Serial.print(".");
        #endif
        state = !state;
        digitalWrite(onboard_led, state);
        delay(500);
    }
    digitalWrite(onboard_led, LOW);
    #ifdef DEBUG
      Serial.println();
    #endif
    if (WiFi.status() != WL_CONNECTED){
        #ifdef DEBUG
          Serial.println("Nao foi possivel se conectar a rede");
        #endif
        return false;
    }
    #ifdef DEBUG
      Serial.println("Conexao estabelecida");
    #endif
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);   
    return true;
}

void handle_register(){
  if (server.hasArg("ssid") && server.hasArg("password")){
    sta_ssid = server.arg("ssid");
    sta_password = server.arg("password");
    file = LITTLEFS.open("/credentials.txt", "w");
    file.print(sta_ssid + ":" + sta_password);
    file.close();
    #ifdef DEBUG
      Serial.println("Dados salvos. Reiniciando o microcontrolador...");
    #endif
    delay(3000);
    ESP.restart();
  }
   server.send(200, "text/html", WebPage); 
}
void handle_not_found(){
  server.send(404, "text/plain", "Not found");    
}
void register_device(){
  digitalWrite(onboard_led, HIGH);
  start_ap_sta();
  server.on("/", handle_register);
  server.onNotFound(handle_not_found);
  server.begin();                                                                 //Inicia o servidor
  while (true) { 
    server.handleClient();                                                           //Executa as ações do servidor
  }
}
void init_device(){
  #ifdef DEBUG
    Serial.println(LITTLEFS.exists("/index.html"));
  #endif
  if (LITTLEFS.exists("/credentials.txt")){
    file = LITTLEFS.open(("/credentials.txt"), "r");
    String credentials = file.readString();
    file.close();
    sta_ssid = credentials.substring(0, credentials.indexOf(":"));
    sta_password = credentials.substring(credentials.indexOf(":")+1);
    #ifdef DEBUG
      Serial.print("SSID lido: ");
      Serial.println(sta_ssid);
      Serial.print("Password lido: ");
      Serial.println(sta_password);
    #endif
    if(!start_sta()){
      LITTLEFS.remove("/credentials.txt");
      ESP.restart();
  }
  }
  else{
    register_device();
  }
}
