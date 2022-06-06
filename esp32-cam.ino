#include "esp_camera.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"



// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

int contador=0;

//Bola Roja

int x=20;
int y=20;

int ax=20;
int ay=10;

//Bola Verde

float Rposx=160;
float Rposy=120;
int Rpuntos=0;

float Gposx=160;
float Gposy=120;
int Gpuntos=0;

float Bposx=160;
float Bposy=120;
int Bpuntos=0;
    

void setup(void) {
  
  Serial.begin(115200);
  
  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);
  
        
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector


  ///Cargamos la cámara
        
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  //config.pixel_format = PIXFORMAT_RGB565; 
  config.pixel_format = PIXFORMAT_RGB888; 
  //config.pixel_format = PIXFORMAT_GRAYSCALE;
  //config.pixel_format = PIXFORMAT_JPEG;
  
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_QQVGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_XGA;
    config.jpeg_quality = 10;
    config.fb_count = 1;
  }
  
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
 
  //delay(5000);
  //Serial.println("TURMANCV");
     
}


void loop() {

  camera_fb_t * fb = esp_camera_fb_get();
  
  if (fb){
    
   // Serial.print("Width : ");Serial.println(fb->width);
   // Serial.print("Height : ");Serial.println(fb->height);
   // Serial.print("Length : ");Serial.println(fb->len);
    
    int cont=0;     

    int contador=0;

    Rposx=0;
    Rposy=0;
    Rpuntos=0;

    Gposx=0;
    Gposy=0;
    Gpuntos=0;
    
    Bposx=0;
    Bposy=0;
    Bpuntos=0;

    for(int y = 0; y < 120; y++){    
        for(int x = 0; x < 160; x++){ 
           

            int b = fb->buf[cont];cont++;
            int g = fb->buf[cont];cont++;
            int r = fb->buf[cont];cont++;
            
            
            if((r>g+20)&&(r>b+20)){               
                  Rposx+=(160-x);
                  Rposy+=y;      
                  Rpuntos++;               
            }
            
            if((g>r+20)&&(g>b+20)){               
                  Gposx+=(160-x);
                  Gposy+=y;      
                  Gpuntos++;               
            }

            if((b>r+20)&&(b>g+20)){               
                  Bposx+=(160-x);
                  Bposy+=y;      
                  Bpuntos++;               
            }
            
        }
       
    }

    if(Rpuntos==0){ Rposx=0;Rposy=0; }else { Rposx=Rposx/Rpuntos;Rposy=Rposy/Rpuntos; }
    if(Gpuntos==0){ Gposx=0;Gposy=0; }else { Gposx=Gposx/Gpuntos;Gposy=Gposy/Gpuntos; }
    if(Bpuntos==0){ Bposx=0;Bposy=0; }else { Bposx=Bposx/Bpuntos;Bposy=Bposy/Bpuntos; }
        
    esp_camera_fb_return(fb);   
    
  }

  String peticion="";
  while(Serial.available()){
    char c=Serial.read();
    peticion+=c;    
  }

  if(peticion!=""){  
    if(peticion.indexOf("RED")!=-1){
        String dato="";dato+=Rposx;dato+=";";dato+=Rposy;dato+=";";dato+=Rpuntos;dato+=";";    
        Serial.println(dato);                  
    }else if(peticion.indexOf("GREEN")!=-1){
        String dato="";dato+=Gposx;dato+=";";dato+=Gposy;dato+=";";dato+=Gpuntos;dato+=";";    
        Serial.println(dato);                  
    }else if(peticion.indexOf("BLUE")!=-1){
        String dato="";dato+=Bposx;dato+=";";dato+=Bposy;dato+=";";dato+=Bpuntos;dato+=";";    
        Serial.println(dato);                  
    }    
  }
 
  
}
