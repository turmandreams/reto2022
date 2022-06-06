#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

#include <Wire.h>
#include "MPU6050.h"
#include <EEPROM.h>

#include "Adafruit_VL53L0X.h"
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
VL53L0X_RangingMeasurementData_t measure;


   
#define MPU 0x68

int16_t ax=10;
int16_t ay=20;
int16_t az=30;
int16_t gx=0;
int16_t gy=0;
int16_t gz=0;
int16_t temp=25;

int16_t dt=0;
int16_t tiempo_prev=0;


float girosc_ang_z=0;
float girosc_ang_z_prev=0;


const int motorI1=12;
const int motorI2=14;
const int motorD1=27;
const int motorD2=26;

const int pindisparo=25;
const int pincinta=33;

const int encodercinta=34;

const int umbralpelota=200;
const int umbralrecoge=50;

float angulo=0;
float angulomin=0;
int valormin=0;

String dato="";

float posx=0;
float posy=0;
int numpuntos=0;

int umbralpuntos=50;

const int umbralpuntosrojo=40;
const int umbralpuntosazul=70;


int bolascogidas=0;
boolean cogida=false;
boolean encontrada=false;

boolean buscardiana=false;

byte numdisparos=0;

void carga() {

    EEPROM.begin(512);     
    EEPROM.get(0,numdisparos);
    EEPROM.commit();
    EEPROM.end();      
}

void graba() {
  yield();
  EEPROM.begin(512);            
  EEPROM.put(0,numdisparos);
  EEPROM.end();   
  yield();  
}

void ESPCAM(String solicitud){

   Serial.println(solicitud);
       
   posx=0;
   posy=0;
   numpuntos=0;
          
   int cont=0;
   dato="";
   while(Serial.available()){
            char c=Serial.read();
            if(c==';'){
              if(cont==0){ posx=dato.toFloat();  }
              else if(cont==1){ posy=dato.toFloat();  }
              else if(cont==2){ numpuntos=dato.toInt();  }
              dato="";
              cont++;                
            }else{
              dato+=c;        
            }
    }
        
    dato="";dato+=posx;dato+=" , ";dato+=posy;dato+=" , ";dato+=numpuntos;dato+="\n";
    enviadato();    
    
}


void iniciaMPU(){


   Wire.begin(21,22);
  
   Wire.beginTransmission(MPU);
   Wire.write(0x6B);
   Wire.write(0);
   Wire.endTransmission(true);
  
   Wire.beginTransmission(MPU);
   Wire.write(0x19);   //Sample Rate
   Wire.write(0x00);   //  8000 / 1 + 0
   Wire.endTransmission(true);
  
   Wire.beginTransmission(MPU);
   Wire.write(0x1C);   //Setting Accel
   Wire.write(0x18);      //  + / - 16g
   Wire.endTransmission(true);

  
}

int leeVL53L0X(){
  lox.rangingTest(&measure, false); 
  
  if(measure.RangeStatus != 4) {
      int valor=measure.RangeMilliMeter;
      if(valor==0) { return -1; }
      return valor;      
  }
  return -1;
}

void leempu6050() {

  Wire.beginTransmission(MPU);
  Wire.write(0x47); //Pedir el registro 0x3B - corresponde al AcX
  Wire.endTransmission(false);
  Wire.requestFrom(MPU,2,true); //A partir del 0x3B, se piden 6 registros
  
  while(Wire.available() < 2);
    
  gz=Wire.read()<<8|Wire.read();

  Wire.endTransmission(true);


  ///Actualizamos Angulo

  dt = millis() - tiempo_prev;
  tiempo_prev = millis();
  girosc_ang_z = (gz / 131)*dt / 1000.0 + girosc_ang_z_prev;
  girosc_ang_z_prev = girosc_ang_z;
  angulo=girosc_ang_z;

  // dato="Angulo : ";dato+=angulo;dato+="\n";   
  // Serial.println(dato);  
  // enviadato();
  
}

void enviadato(){

  //Serial.println(dato);
  
  int str_len = dato.length() + 1; 
  char char_array[str_len];  
  dato.toCharArray(char_array, str_len);
  for(int i=0;i<str_len;i++){
      SerialBT.write(char_array[i]);  
  }
  
}

void cinta(){

  digitalWrite(pincinta,HIGH);
  delay(300);
  float valorcinta=0;
  do{                     
    float valorc=analogRead(encodercinta);
    valorcinta=(valorcinta*0.9)+(valorc*0.1);
    //Serial.println(valorcinta);
    delay(1);    
  }while(valorcinta<2000);
                       
  digitalWrite(pincinta,LOW);              

  umbralpuntos=umbralpuntosrojo;
  
                  
}

void setup() {
  

  Serial.begin(115200);
  SerialBT.begin("ROBOT_PINPON"); //Bluetooth device name

  iniciaMPU(); 
  
  pinMode(pindisparo,OUTPUT);digitalWrite(pindisparo,LOW);
  pinMode(pincinta,OUTPUT);digitalWrite(pincinta,LOW);
  
  ledcAttachPin(motorI1,0);ledcSetup(0,30,8);
  ledcAttachPin(motorI2,1);ledcSetup(1,30,8);
  ledcAttachPin(motorD1,2);ledcSetup(2,30,8);
  ledcAttachPin(motorD2,3);ledcSetup(3,30,8);

  ledcWrite(0,0);ledcWrite(1,0);
  ledcWrite(2,0);ledcWrite(3,0);
  
 
  if(!lox.begin()) {
    //Serial.println(F("Failed to boot VL53L0X"));
    delay(10000);
    while(1);
  } 

  delay(200);

  carga();

  if(numdisparos!=0){
     delay(1000); 
     if(numdisparos>0) { numdisparos--; }     
     graba();
     digitalWrite(pindisparo,HIGH);delay(50);digitalWrite(pindisparo,LOW);ESP.restart();     
     
  }else{  
    ledcWrite(0,0);ledcWrite(1,100);  
    ledcWrite(2,0);ledcWrite(3,100);
  }
}


void loop() {

  if(!buscardiana){ ESPCAM("RED"); }
  else{ ESPCAM("BLUE"); }

  leempu6050();
  
  if(angulo>360){
      girosc_ang_z=0;
      girosc_ang_z_prev=0;
      if(umbralpuntos>10){umbralpuntos-=10;}
  }

  if(posx!=0){
  
      if((posx>70)&&(posx<90)&&(numpuntos>umbralpuntos)){  
          ledcWrite(0,0);ledcWrite(1,100);      
          ledcWrite(2,100);ledcWrite(3,0); 
          encontrada=true;
          girosc_ang_z=0;
          girosc_ang_z_prev=0;
      }else{
          if(encontrada){
            ledcWrite(0,255);ledcWrite(1,0);      
            ledcWrite(2,255);ledcWrite(3,0);       
            delay(50);    
          }
          encontrada=false;
          
          ledcWrite(0,0);ledcWrite(1,100);  
          ledcWrite(2,0);ledcWrite(3,100);
      }

  }

  if(!buscardiana){    
  
    int valor=leeVL53L0X();
  
    if((valor<umbralpelota)&&(valor>0)){        
          
          ledcWrite(0,0);ledcWrite(1,0);
          ledcWrite(2,0);ledcWrite(3,0);
    
          dato="Distancia : ";dato+=valor;dato+="\n";
           
          // Serial.println(dato);
          // enviadato();
                  
          do{
                  ledcWrite(0,0);ledcWrite(1,100);      
                  ledcWrite(2,100);ledcWrite(3,0);                            
                  delay(50);    
  
                  ledcWrite(0,0);ledcWrite(1,0);
                  ledcWrite(2,0);ledcWrite(3,0);                  
                  
                  valor=leeVL53L0X();                
                  if(valor>200) { break; }                 
                  
           }while(valor>umbralrecoge);
        
           if(valor<umbralrecoge){                       
                
                  ledcWrite(0,0);ledcWrite(1,255);  
                  ledcWrite(2,0);ledcWrite(3,0); 

                  cinta();
                  
                  girosc_ang_z=0;
                  girosc_ang_z_prev=0;
                  bolascogidas++;
                  if(bolascogidas>=5){ 
                    buscardiana=true; 
                    umbralpuntos=umbralpuntosazul;
                  }
                  
            }              
              
          
    }   
  
  }else{
    
      int valor=leeVL53L0X();
  
      if((valor<umbralpelota)&&(valor>0)){  
        
        ledcWrite(0,0);ledcWrite(1,0);  
        ledcWrite(2,0);ledcWrite(3,0); 
        delay(100);
        
        ledcWrite(0,255);ledcWrite(1,0);      
        ledcWrite(2,0);ledcWrite(3,255); 
        delay(200);

        ledcWrite(0,0);ledcWrite(1,0);  
        ledcWrite(2,0);ledcWrite(3,0); 
        delay(100);
        
        girosc_ang_z=0;
        girosc_ang_z_prev=0;
        leempu6050();
        
        ledcWrite(0,0);ledcWrite(1,170);  
        ledcWrite(2,0);ledcWrite(3,170); 

        do{
            leempu6050();
            delay(10);
        }while(angulo<180);
                  
        ledcWrite(0,0);ledcWrite(1,0);  
        ledcWrite(2,0);ledcWrite(3,0); 
        
        delay(1000);
        numdisparos=4;
        graba();
        digitalWrite(pindisparo,HIGH);delay(50);digitalWrite(pindisparo,LOW);ESP.restart(); 

        
      }
    
  }
    
 
  
}
