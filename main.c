#include "header.h"

#define out_max_x       127
#define out_max_y       63
#define out_min_x       0
#define out_min_y       0

#define in_min          0
#define in_max          4095

void vykresleni( void *pvParameters );
void tlacitka( void *pvParameters );

int my_map_x(int in);
int my_map_y(int in);

void check_buttons();
void action();

int buttons[2] = {16,17};
int buttonState[2];
int lastButtonState[2];
boolean buttonIsPressed[2];
unsigned long lastDebounceTime = 0;  
const uint8_t debounceDelay = 50;
bool kresleni;
int pot1, pot2;
int pos_x, pos_y, prev_x, prev_y;

SemaphoreHandle_t xSemaphore;

void setup() {
  
  // initialize serial communication at 115200 bits per second:
  Serial.begin(9600);
  
  ST_Init();
  ST_GraphicsON();
  ClearScreen();
  
  pinMode(15, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  
  for (uint8_t i = 0; i < 2; i++) {
    pinMode(buttons[i], INPUT_PULLUP);
    lastButtonState[i] = LOW;
    buttonIsPressed[i] = false; 
  }

  TaskHandle_t f1_h = NULL;
  TaskHandle_t f2_h = NULL;
  
  xSemaphore = xSemaphoreCreateMutex();
  kresleni = true;
  prev_x = prev_y = 0;
  
  xTaskCreatePinnedToCore( vykresleni ,"kresleni" ,2048, NULL , 1, &f1_h, 0);
  xTaskCreatePinnedToCore( tlacitka ,"tlacitka" ,2048, NULL , 1, &f2_h, 1);
}

void loop(){
  // Empty. Things are done in Tasks.
   
}



void vykresleni(void *pvParameters)  
{
  (void) pvParameters;
  
  for(;;){
    
     pot1 = analogRead(15);
     pot2 = analogRead(4);
  
     pos_x = my_map_x(pot2);
     pos_y = my_map_y(pot1);

     pos_x += prev_x;
     pos_y += prev_y;

     pos_x /= 2;
     pos_y /= 2;

     

    if(kresleni){
      SetPixel(pos_x, pos_y);
    }else{
      SetBarvaOFF();
      SetPixel(prev_x, prev_y);
      SetBarvaON();
      SetPixel(pos_x, pos_y);
    }
   
    action();

    prev_x = pos_x;
    prev_y = pos_y;
    
    //Serial.println("vykresleni");
    vTaskDelay(1 / portTICK_PERIOD_MS); 
  }
}

void tlacitka(void *pvParameters)  
{
  (void) pvParameters;
  
  for(;;){
    
    check_buttons();
    //action();
    
    
    //Serial.println("martin pitin");
    vTaskDelay(1 / portTICK_PERIOD_MS);  // 100ms delay
  }
}

int my_map_x(int in){
  return in * (out_max_x - out_min_x + 1) / (in_max - in_min + 1) + out_min_x; 
}

int my_map_y(int in){
  return in * (out_max_y - out_min_y + 1) / (in_max - in_min + 1) + out_min_y; 
}

void check_buttons() {
  for (uint8_t currentButton=0; currentButton < 2; currentButton++) {
    
    int reading = digitalRead(buttons[currentButton]); 
    
    if (reading != lastButtonState[currentButton])
        lastDebounceTime = millis();
    
    if ((millis() - lastDebounceTime) > debounceDelay) {
     
      if (reading != buttonState[currentButton]) {
        buttonState[currentButton] = reading;
        
        if (buttonState[currentButton] == HIGH) { 
          //mutex
          if(xSemaphore != NULL){
            if(xSemaphoreTake(xSemaphore, (TickType_t) 10) == pdTRUE){
              //Serial.println("mutex core 1");
               buttonIsPressed[currentButton] = true;
               xSemaphoreGive(xSemaphore);
            }//else neco
          }   
          //buttonIsPressed[currentButton] = true;                
        }
      }
    }
    lastButtonState[currentButton] = reading;
  }
}

/*void action(){
  for (uint8_t currentButton=0; currentButton < 2; currentButton++) {
    //mutex
    if(buttonIsPressed[currentButton]){
      switch(currentButton){
        case 0:     //kresleni, nastaveni
        kresleni = !kresleni;
        Serial.println("2");
        buttonIsPressed[currentButton] = false;
        break;

        case 1: // smazani a nastaveni
        
        //ClearScreen();
        kresleni = false;
        Serial.println("3");
        buttonIsPressed[currentButton] = false;
        break;        
      }
    }
    //mutex
  }
}*/


void action(){
  for (uint8_t currentButton=0; currentButton < 2; currentButton++) {
    //mutex
    if(xSemaphore != NULL){
       if(xSemaphoreTake(xSemaphore, (TickType_t) 10) == pdTRUE){          
          if(buttonIsPressed[currentButton]){
             switch(currentButton){
                   case 0:     //kresleni, nastaveni
                          kresleni = !kresleni;
                          //Serial.println("2");
                          buttonIsPressed[currentButton] = false;
                  break;

                  case 1: // smazani a nastaveni       
                          ClearScreen();
                          kresleni = false;
                          //Serial.println("3");
                          buttonIsPressed[currentButton] = false;
                  break;        
             }
          }              
          xSemaphoreGive(xSemaphore);
        }//else neco
    }    
  }
}
