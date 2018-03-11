
#include <Wire.h>
#include <SPI.h>
#include <toneAC.h> 
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#define pressure_0 1015.25  // Pression QNH du jour
#define alt_max 1700    // Altitude maximal (permet de faire une alerte : dépassement de la TMA ...)

// Pins utilises, le buzzer sont sur les pins 9 & 10 pour l'atmega 328 (voir librairie toneAC)
#define vibrant 12
#define button 3

Adafruit_BMP280 bme;  // cree objet barometre

float alt = 1000, alt_old = 1000; // valeur initiale permettant de converger plus vite vers la vraie valeur
float t, t_old = millis();
float vario = 0;

// vitesses entre lequels le variometre ne fera pas de bips 
const float vario_climb_start = 0; 
const float vario_sink_start = -1.3; 

// variables pour le bouton
bool state = 0;         
bool buttonState;             
bool lastButtonState = LOW;   
unsigned long lastDebounceTime = 0;  
unsigned long debounceDelay = 50; 


float timer = millis();
uint8_t beepLatency;
uint16_t beepFrequency;


void updateAlt(){
  alt = alt * 0.99 + 0.01 * bme.readAltitude(pressure_0);
}

void updateVario(){
  vario = vario * 0.99 + 10000 * ((alt-alt_old)/(t-t_old));
}


void vibrate(){
  if (vario > vario_climb_start)
    digitalWrite(vibrant, 1);
  else
    digitalWrite(vibrant, 0);
}


void button_check(){

  bool reading = digitalRead(button);
  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {

    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == HIGH) {
        state = !state;
      }
    }
  }
  lastButtonState = reading;
}


uint16_t getBeepFrequency(float vario)
{
  int frequency = 690 + (120 * vario);
  return (frequency < 200) ? 200 : (frequency > 1300) ? 1300 : frequency;
}

uint8_t getBeepLatency(float vario)
{
  int latency = 200 - (vario * 26);
  return (latency < 70) ? 70 : (latency > 200) ? 200 : latency;
}


void makeBeeps(){
    digitalWrite(vibrant, 0);
    if (vario >= vario_climb_start || vario <= vario_sink_start){
      beepLatency = getBeepLatency(vario);
      beepFrequency = getBeepFrequency(vario);
    
      if ( millis()>= timer  + beepLatency*4){
        toneAC (beepFrequency, 10, beepLatency/2);
        timer = millis();    
    }
  }
}


void setup() {

  pinMode(vibrant, OUTPUT);
  pinMode(button, INPUT_PULLUP);
  
 if (!bme.begin()){ // initialisation du baromètre 
  while (1){
    digitalWrite(LED_BUILTIN, HIGH);   
    delay(1000);                      
    digitalWrite(LED_BUILTIN, LOW);    
    delay(1000);   
  }
 }
}



void loop() {
  t = micros();
  updateAlt();
  button_check();
  updateVario();
  
  if(alt > alt_max)
    toneAC(3000);
  else{
    if (state)
      makeBeeps();
    else
      vibrate();
  }
  alt_old = alt;
  t_old = t; 
  

}
