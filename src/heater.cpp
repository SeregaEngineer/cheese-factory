#include <Arduino.h>
#include "heater.h"


//Конструктор передаем номер пина и включаем его на выход
Heater::Heater(uint8_t pin)
{
    _pin = pin;
    pinMode(_pin, OUTPUT);
 }

// влючение отключение
  void Heater::oNoff(bool command)
{
  if (command)digitalWrite(this->_pin, HIGH);
  else if(!command) digitalWrite(this->_pin, LOW);  
}


//Поддержание температуры на заднном уровне // гистерезис в флоат если памяти будет хватать
void Heater::tempMaint (float *temp, uint8_t setpoint, uint8_t hyster){
  if(*temp < (setpoint - hyster)){
        oNoff(true);
   }
  else if (*temp > (setpoint + hyster))  {
        oNoff(false);
    }
    
  }

//Нагреть до
void Heater::heatTo(float* temp, uint8_t setpoint, bool *flag){
    if(*temp < setpoint && *flag){
        oNoff(true);
    }
    else if(*temp > setpoint)
    {
       oNoff(false);
       *flag = false;
    }
   }



Heater::~Heater()
{
}

    