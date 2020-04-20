#include <Arduino.h>
#include "readT.h"
#include "getTime.h"
#include <TimeLib.h>
#include <Wire.h>
#include <DS1307RTC.h>


unsigned long currentTime = 0; // текущее время работы
long previousMillis = 0; // предыдущение сработка       
long previousMillis1 = 0;
long previousMillis11 = 0;
long previousMillis_read = 0;
long time_click = 0;
float temp;
uint8_t gister = 0.3; //гистеререзис
uint8_t step = 0;
const uint8_t heater = 13; // Пин подключение нагревателя
const uint8_t btn_on = 2;  //Пин подключения кнопки
const uint8_t buz = 3;  //Пин для подключения пищалки
const uint8_t motor = 4;  //Пин для подключения пищалки
uint8_t setTemp1 = 27; // Уставка по температуре 1
uint8_t setTemp2 = 37;
bool buz_status = true; //для единичного вклчюения буззера на стадии нагрева
uint32_t start_time; // переменны для записи начанало поддержания температуры
bool var = false; // Для  для включения сервы после нажатия на кнопку
bool varHeatTo = false; //перемннная для функции нагреть до

//включение тена 
void controlHeater(bool ON){
    if (ON)digitalWrite(heater, HIGH);
    else if(!ON) digitalWrite(heater, LOW);
}

// функция поддерживания температуры // Сделать класс нагревателя
void step_heating(uint8_t setpoint){
  if(temp < setpoint - gister){
   controlHeater(1);
   }
  else if (temp > setpoint + gister)  {
    controlHeater(0);
  }
}

//Нагреть до
void heatTo(uint8_t setpoint){
   if(temp < setpoint - gister  && varHeatTo){
   controlHeater(1);
   }
   else if (temp > setpoint + gister)  {
    controlHeater(0);
    varHeatTo = false;
  }

}

//включение выключение движка
void controlMotor(bool ON){
     if (ON)digitalWrite(motor, HIGH);
    else if(!ON) digitalWrite(motor, LOW);

}



//зыуковой сигнал
void buzzer(){
  for(uint8_t i = 0; i<=3;i++)
   {
      
    digitalWrite(buz, !digitalRead(buz));
    delay(1000);
  }
}




void setup() {
  pinMode(heater, OUTPUT);
  pinMode(btn_on, INPUT_PULLUP);
  pinMode(buz, OUTPUT);
  pinMode(motor, OUTPUT);
  Serial.begin(9600);
  while (!Serial) ; // wait until Arduino Serial Monitor opens
  setSyncProvider(RTC.get);   // the function to get the time from the RTC
  if(timeStatus()!= timeSet) 
     Serial.println("Unable to sync with the RTC");
  else
     Serial.println("RTC has set the system time");      
  
}


void loop() { 
currentTime = millis();
 
 if (currentTime - previousMillis > 750) // текущие - предыдущие
 {
   temp = readTemp();
   Serial.println(temp);
   previousMillis = currentTime;
 }   
 

switch (step)
{
case 0:
  if(!digitalRead(btn_on))  step = 1; // ждем нажатия кнопки для запуска
  break;

case 1: //Нагреваем до 37 пищим и погнали дальше
  step_heating(setTemp1); // нагреваем до 37 и поддерживаем температуру
  // При достижение 37 нужно однократно пропикать буззером
  if (temp > setTemp1) 
  {
    buzzer(); 
    start_time = getTimeInMin();
    step ++;
  }
  break;

case 2: //30 митут греем 
    step_heating(setTemp1); // нагреваем до 37 и поддерживаем температуру
    if (currentTime - previousMillis11 > 1000) // текущие - предыдущие
   {
   if(getTimeInMin()-start_time >= 1){
     buzzer(); 
     start_time = getTimeInMin();
     step ++;
     controlHeater(0);
   }
   previousMillis11 = currentTime;
   }   
break;

case 3: //таймер 20 минут
   
    if (currentTime - previousMillis11 > 1000) // текущие - предыдущие
   {
   if(getTimeInMin()-start_time >= 1){
     buzzer(); 
     step ++;
   }
   previousMillis11 = currentTime;
   }   
break;

case 4: //Жднем нажатие кнопки для включение сервы на 30 секунд
      if(!digitalRead(btn_on)){
      start_time = getTimeInSec();   
      var = true;  
    }
    if(var) controlMotor(true);  // нажали на кнопку включем движок
    if (currentTime - previousMillis11 > 1000 && var) // текущие - предыдущие
     {
     if(getTimeInSec() - start_time >= 30){
         controlMotor(false);
         buzzer();  //через 30 сек вырбуаем движок и пищим бузером
   
     var = false;
     step ++;
   }
   previousMillis11 = currentTime;
   }   
break;

case 5:
//выводи сообщения о замене сервы  ждем нажатие кнопки и мешаем 15 минут
   if(!digitalRead(btn_on)){
      start_time = getTimeInMin();   
      var = true;  
      varHeatTo  = true;
    }
    if(var) {
      controlMotor(true);  // нажали на кнопку включем движок
      heatTo(27);             //и нагреваем до 42
    }
    if (currentTime - previousMillis11 > 1000 && var) // текущие - предыдущие
     {
     if(getTimeInMin() - start_time >= 2){
    controlMotor(false);
     buzzer();  //через 15 минут вырбуаем движок и пищим бузером
     var = false;
     controlHeater(false);
     start_time = getTimeInMin();
     step ++;
   }
   previousMillis11 = currentTime;
   }  

break;
// нагрев до 50  и 1.5 держим температуру
case 6:
step_heating(27);//греем до 50 и ждем 1.5 часа
if(getTimeInMin() - start_time >= 2){
     buzzer();  //через 15 минут вырбуаем движок и пищим бузером
     var = false;
     controlHeater(false);
     start_time = getTimeInMin();
     step ++;
   }

break;

case 7: //STOP
controlHeater(false);
controlMotor(false);
break;
}
}