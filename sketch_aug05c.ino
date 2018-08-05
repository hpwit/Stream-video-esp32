 




#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#define USE_SPI 1
#define FASTLED_ALLOW_INTERRUPTS 0
#define INTERRUPT_THRESHOLD 1
#include "FastLED.h"
FASTLED_USING_NAMESPACE
#define FASTLED_SHOW_CORE 0





WiFiUDP Udp2;


#define PORT_MASK 0b110111011111111000000110100  //0b1001111111000000111101
#define NUM_STRIPS 16



#define LED_WIDTH 123
#define LED_HEIGHT_PER_STRIP 3
#define LED_HEIGHT NUM_STRIPS*LED_HEIGHT_PER_STRIP
#define NUM_LEDS NUM_STRIPS * NUM_LEDS_PER_STRIP
#define NUM_LEDS_PER_STRIP LED_HEIGHT_PER_STRIP*LED_WIDTH
;

CRGB leds[NUM_LEDS+1];
CRGB Tpic[NUM_LEDS+1];

char *artnetPacket2;



// Artnet settings
//Artnet artnet;

//this function is only used for me 'cause half of my strip are GRB instead of RGB

void replaceled()
{
  int offset=0;
  for(int i=0;i<123;i++)
 {
   byte s=leds[i+offset].g;
  // char buff[9];
   // my_itoa (s,buff,16,8);
    //Serial.println(buff);
   leds[i+offset].g= leds[i+offset].r;
   leds[i+offset].r= s;
   //CRGB((s&0x0F000)>>8,(s&0x00FF0000)>>16 ,s & 0xFF) ;  //(leds[i+offset] & 0xFF) |  ( (leds[i+offset] & 0x00FF00L)<<8   ) |  (  (leds[i+offset] & 0xFF0000L)>>8  );
 }
 offset=24*LED_WIDTH;
  for(int i=0;i<24*LED_WIDTH;i++)
 {
   byte s=leds[i+offset].g;
  // char buff[9];
   // my_itoa (s,buff,16,8);
    //Serial.println(buff);
   leds[i+offset].g= leds[i+offset].r;
   leds[i+offset].r= s; 
   //CRGB((s&0x0F000)>>8,(s&0x00FF0000)>>16 ,s & 0xFF) ;  //(leds[i+offset] & 0xFF) |  ( (leds[i+offset] & 0x00FF00L)<<8   ) |  (  (leds[i+offset] & 0xFF0000L)>>8  );
 }
 //on met les boards en noir

 for (int i=0;i<LED_HEIGHT;i++)
 {
  leds[i*LED_WIDTH]=CRGB::Black;
  leds[(i+1)*LED_WIDTH-1]=CRGB::Black;
 }
}


CRGB artnetled[32*32];





//manage the the core0


// -- Task handles for use in the notifications

static TaskHandle_t FastLEDshowTaskHandle2 = 0;
static TaskHandle_t userTaskHandle = 0;





void FastLEDshowESP322()
{
    if (userTaskHandle == 0) {
        const TickType_t xMaxBlockTime = pdMS_TO_TICKS( 200 );
        // -- Store the handle of the current task, so that the show task can
        //    notify it when it's done
       // noInterrupts();
        userTaskHandle = xTaskGetCurrentTaskHandle();
        
        // -- Trigger the show task
        xTaskNotifyGive(FastLEDshowTaskHandle2);
        //to thge contrary to the other one we do not wait for the display task to come back
    }
}



void FastLEDshowTask2(void *pvParameters)
{
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS( 500 );
    // -- Run forever...
    for(;;) {
        // -- Wait for the trigger
        ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
        
            
           
                memcpy(leds,Tpic,LED_WIDTH*LED_HEIGHT*sizeof(CRGB));
            
            replaceled();
            FastLED.show();
            
               userTaskHandle=0; //so we can't have two display tasks at the same time
                 
           }
}




void fill(CRGB color)
{
     fill_solid(leds, NUM_LEDS, color);
    fill_solid(Tpic, NUM_LEDS, color);
}


bool firsttime=true;


  uint32_t syncmax1=0;
  uint32_t syncmax2=0;
void setup() {
   Serial.begin(115200);
 
 

 
  xTaskCreatePinnedToCore(FastLEDshowTask2, "FastLEDshowTask2", 1000, NULL,3, &FastLEDshowTaskHandle2, FASTLED_SHOW_CORE);
 
  FastLED.addLeds<WS2811_PORTA,NUM_STRIPS,PORT_MASK>(leds, NUM_LEDS_PER_STRIP);
 
  FastLED.setBrightness(64);

  fill(CRGB::Black);
  FastLED.show();
  FastLED.delay(2000);
  
   


 WiFi.mode(WIFI_STA);
    Serial.printf("Connecting to %s\n", "WiFi-2.4-E19C");
    Serial.printf("Connecting ");
    WiFi.begin("WiFi-2.4-E19C", "yvesyves");
//WiFi.begin("DomainePutterie", "Jeremyyves");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.println(WiFi.status());
      
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    
     //server.begin();
   
int nbNeededUniverses=LED_HEIGHT/2;
    if(nbNeededUniverses<=32)
    {
        if(nbNeededUniverses<32)
            syncmax1=(1<<nbNeededUniverses)-1;
        else
            syncmax1=0xFFFFFFFF;
        syncmax2=0;
    }
    else
    {
            syncmax1=0xFFFFFFFF;
        if(nbNeededUniverses-32<32)
            syncmax2=(1<<(nbNeededUniverses-32))-1;
        else
            syncmax2=0xFFFFFFFF;
            //syncmax2=0;

    }

Udp2.begin(100);
 artnetPacket2=(char*)malloc((LED_WIDTH*3*2+1)*sizeof(char));
 if(artnetPacket2==NULL)
  {
    Serial.println("impossible to create buffer");
   return;
  }
  else
  {
    Serial.println("buffer créé");
  }
     
     
}



void loop() {
  
  uint32_t sync=0;
  uint32_t sync2=0;

 while (sync!=syncmax1 or sync2!=syncmax2)//sync!=syncmax or sync2!=syncmax2 
 { 
int packetSize = Udp2.parsePacket();
      if(packetSize>0)
      {
       
        
      Udp2.read(artnetPacket2, packetSize);
      memcpy(&Tpic[LED_WIDTH*2*(artnetPacket2[0])],artnetPacket2 + 1,LED_WIDTH*3*2);
     //Serial.printf("univers:%d\n",artnetPacket2[0]);
      if(artnetPacket2[0]==255)
      {
        Serial.printf("new value bru:%d\n",artnetPacket2[1]);
        FastLED.setBrightness(artnetPacket2[1]);
      }
      else
      {
            if (artnetPacket2[0]==0)
            {
              sync=1;
              sync2=0;
            }
             else
             {
               if(artnetPacket2[0]<32)
                 sync=sync  | (1<<artnetPacket2[0]);
               else
               sync2=sync2  | (1<<(artnetPacket2[0]-32));
             
             }
      } 
      //free(artnetPacket);
      }
 
    }
 

 FastLEDshowESP322();




}





