#include <Arduino.h>

class Heater {
private:
    uint8_t _setpoint;
    uint8_t _hyster;
    uint8_t _pin;


public:
//Конструктор передаем номер пина и включаем его на выход

Heater(uint8_t);

// влючение отключени
void oNoff(bool command);


//Поддержание температуры на заднном уровне
void tempMaint (float *temp, uint8_t setpoint, uint8_t hyster);


//Нагреть до
void heatTo(float* temp, uint8_t setpoint, bool* flag);

~Heater();


};