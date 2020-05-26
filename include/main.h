#include "readT.h"
#include "getTime.h"
#include <TimeLib.h>
#include <Wire.h>
#include <DS1307RTC.h>
#include "heater.h"
#include "GyverEncoder.h"
#include <LiquidCrystal_I2C.h>


void menuSwipe(int8_t *menu);  // Переход между выбором режима пастеризация/сыр/стоп
void controlMotor(bool ON); // Управление мотором вкл, выкл
void buzzer();      //Звуковой сигнал, Надо бы убрать delay()
void change_setpoint(int8_t *temperatura, int8_t *time);

bool btContinue(int8_t time_process);// Функция кнопки продолжить, переход на следующий шаг
void exitInStep(int8_t *temperatura, int8_t *time); // Выход из шага, сбрасываем флаги, и выключаем нагрузку

void modeCheese(); // Режим готовки сыра
void modeStop(); //Стоп
void modePaster(); // Пастеризация'

void timerMin(); // Таймер