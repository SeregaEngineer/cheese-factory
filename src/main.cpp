#include <Arduino.h>
#include "main.h"

unsigned long currentTime = 0; // текущее время работы
long previousMillis = 0;       // предыдущение сработка
long previousMillis1 = 0;
long previousMillis11 = 0;
long previousMillis_read = 0;
long time_click = 0;
float temp;
uint8_t gister = 0.3; //гистеререзис
uint8_t step = 0;
const uint8_t heater = 13; // Пин подключение нагревателя
const uint8_t btn_on = 2;  //Пин подключения кнопки
const uint8_t buz = 3;     //Пин для подключения пищалки
const uint8_t motor = 4;   //Пин для подключения пищалки
const uint8_t valve = 7;   //Пин для подклчючения клапана
uint8_t setTemp1 = 27;     // Уставка по температуре 1
uint8_t setTemp2 = 37;
bool buz_status = true; //для единичного вклчюения буззера на стадии нагрева
uint32_t start_time;    // переменны для записи начанало поддержания температуры
bool var = false;       // Для  для включения сервы после нажатия на кнопку
bool varHeatTo = false; //перемннная для функции нагреть до
uint8_t menu = 0;       // переменная для работы меню
Encoder enc(10, 9, 8);
LiquidCrystal_I2C lcd(0x3F, 16, 2);
Heater heat(heater);

void setup()
{
  lcd.init(); // initialize the lcd
  // Print a message to the LCD.
  lcd.backlight();

  enc.setType(TYPE2);
  varHeatTo = true;
  pinMode(btn_on, INPUT_PULLUP);
  pinMode(buz, OUTPUT);
  pinMode(motor, OUTPUT);
  Serial.begin(9600);
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(F("stop"));
  while (!Serial)
    ;                       // wait until Arduino Serial Monitor opens
  setSyncProvider(RTC.get); // the function to get the time from the RTC
  if (timeStatus() != timeSet)
    Serial.println("Unable to sync with the RTC");
  else
    Serial.println("RTC has set the system time");
}

void loop()
{
  currentTime = millis();
  enc.tick();

  if (enc.isRightH()) // Добавить условия со step что если тема мутится не надо менять рецпты на ходу
  {
    menu++;
    if (menu > 2)
    {
      menu = 0;
    }
    menuSwipe(&menu);
  }

  if (enc.isLeftH())
  {
    menu--;
    if (menu < 0)
    {
      menu = 2;
    }
    menuSwipe(&menu);
  }

  if (currentTime - previousMillis > 750) // текущие - предыдущие
  {
    temp = readTemp();
    Serial.println(temp);
    lcd.setCursor(0, 0);
    lcd.print(F("T="));
    lcd.setCursor(3, 0);
    lcd.print(temp, 1);

    previousMillis = currentTime;
  }

  switch (menu)
  {
  case 0:
    modeStop();
    break;
  case 1:
    modeCheese();
    break;

  case 2:
    modePaster();
    break;
  }
}

/*  switch (step)
  {

  case -1:
    heat.oNoff(false);
    controlMotor(false);
    digitalWrite(valve, LOW);
    lcd.setCursor(0, 1);
    lcd.print(F("STOP"));
    break;

  case 0:
    heat.oNoff(false);
    controlMotor(false);

    if (btContinue())
    {
      varHeatTo = true;
      step = 1; // ждем нажатия кнопки для запуска
      Serial.println(F("Step 0 done"));
    }
    break;

  case 1: //Нагреваем до T пищим и погнали дальше
    heat.heatTo(&temp, setTemp1, &varHeatTo);
    // При достижение 37 нужно однократно пропикать буззером
    if (temp > setTemp1)
    {
      Serial.println(F("Step 1 done"));
      exitInStep();
    }
    break;

    //шаг два вонести заквски и погреть t мин
  case 2:
    //после внесение за Хз как сделать, можно сразу после нагрведо поддерживать Т или нужно кваски??
    heat.tempMaint(&temp, setTemp1, 1); // нагреваем до Т и поддерживаем температуру

    btContinue();
    if (var)
    {
      //Сообщение о Поддержание температуры или номер шага
      if (currentTime - previousMillis11 > 1000) // текущие - предыдущие
      {
        //каждую сек обеновляем диспей и пишем скольок времени осталось
        if (getTimeInMin() - start_time >= 1)
        { // время для отладки везде порядка минуты
          heat.oNoff(false);
          exitInStep();
          Serial.println(F("Step 2 done"));
        }
        previousMillis11 = currentTime;
      }
    }
    break;
    //Шаг 3 вносим фермент и время t
  case 3: //таймер 20 минут
    btContinue();
    if (var)
    {
      if (currentTime - previousMillis11 > 1000) // текущие - предыдущие
      {
        if (getTimeInMin() - start_time >= 1)
        {
          exitInStep();
          Serial.println(F("Step 3 done"));
        }
        previousMillis11 = currentTime;
      }
    }
    break;

  case 4: //Жднем нажатие кнопки для включение сервы на 30 секунд
    if (btContinue())
      start_time = getTimeInSec();

    if (var)
    {
      controlMotor(true);                        // нажали на кнопку включем движок
      if (currentTime - previousMillis11 > 1000) // текущие - предыдущие
      {
        if (getTimeInSec() - start_time >= 30)
        {
          controlMotor(false);
          exitInStep();
          Serial.println(F("Step 4 done"));
        }
        previousMillis11 = currentTime;
      }
    }
    break;

  case 5:
    //выводи сообщения о замене сервы  ждем нажатие кнопки и мешаем 15 минут

    if (var)
    {
      controlMotor(true);                       // нажали на кнопку включем движок
      heat.heatTo(&temp, setTemp1, &varHeatTo); //и нагреваем t

      if (currentTime - previousMillis11 > 1000)
      {
        if (getTimeInMin() - start_time >= 1)
        {
          controlMotor(false);
          buzzer(); //через 15 минут вырбуаем движок и пищим бузером
          exitInStep();
          Serial.println(F("Step 5 done"));
        }
        previousMillis11 = currentTime;
      }
    }
    break;

  // нагрев до T  и t держим температуру
  case 6:
    heat.tempMaint(&temp, setTemp1, 1); //греем до 50 и ждем 1.5 часа
    controlMotor(true);
    if (currentTime - previousMillis11 > 1000)
    {
      if (getTimeInMin() - start_time >= 2)
      {
        exitInStep();
        step = -1;
        Serial.println(F("Step 6 done"));
      }
      previousMillis11 = currentTime;
    }
    break;

  case 7: // Пастеризация
    btContinue();
    if (var)
    {

      heat.heatTo(&temp, setTemp1, &varHeatTo);
      if (!varHeatTo)
      {
        digitalWrite(valve, HIGH);
        if (temp <= 26)
        {
          digitalWrite(valve, LOW);
          step = -1;
          var = false;
          varHeatTo = true;
          lcd.setCursor(6, 1); // по функционалу но вроде безопаснее
          lcd.print(F("done"));
          buzzer();
        }
      }
    }
  }
}
*/
//включение выключение движка
void controlMotor(bool ON)
{
  if (ON)
    digitalWrite(motor, HIGH);
  else if (!ON)
    digitalWrite(motor, LOW);
}

//зыуковой сигнал
void buzzer()
{
  for (uint8_t i = 0; i <= 3; i++)
  {

    digitalWrite(buz, !digitalRead(buz));
    delay(1000);
  }
}

// кнопка продолжить
bool btContinue()
{
  if (!digitalRead(btn_on))
  {
    start_time = getTimeInMin();
    var = true;

    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("step"));
    lcd.setCursor(5, 1);
    lcd.print(step);

    return true;
  }
  return false;
}
//выход из шага
void exitInStep()
{
  buzzer();
  var = false;
  varHeatTo = true;
  step++;
  heat.oNoff(false);   // на выходе из каждго шага выключаем Тен, очень сомнительно
  lcd.setCursor(7, 1); // по функционалу но вроде безопаснее
  lcd.print(F("done"));
}

void modeCheese()
{
  switch (step)
  {
  case 1: //Нагреваем до T пищим и погнали дальше
    if (!var)
    {
      heat.oNoff(false);
      controlMotor(false);
    }

    if (btContinue())
    {
      Serial.print(var);
      varHeatTo = true;
      //step = 1; // ждем нажатия кнопки для запуска
    }
    if (var)
    {
      heat.heatTo(&temp, setTemp1, &varHeatTo);
      if (temp > setTemp1)
      {
        Serial.println(F("Step 1 done"));
        exitInStep();
      }
    }

    //lcd.setCursor(7,1);
    // lcd.print(F("heat"));
    // При достижение 37 нужно однократно пропикать буззером

    break;

    //шаг два вонести заквски и погреть t мин
  case 2:
    //после внесение за Хз как сделать, можно сразу после нагрведо поддерживать Т или нужно кваски??
    // heat.tempMaint(&temp, setTemp1, 1); // нагреваем до Т и поддерживаем температуру
    if (!var)
    {
      lcd.setCursor(0, 1);
      lcd.print(F("add leaven"));
    }
    btContinue();
    if (var)
    {
      heat.tempMaint(&temp, setTemp1, 1);
      //Сообщение о Поддержание температуры или номер шага
      if (currentTime - previousMillis11 > 1000) // текущие - предыдущие
      {
        //каждую сек обеновляем диспей и пишем скольок времени осталось
        if (getTimeInMin() - start_time >= 1)
        { // время для отладки везде порядка минуты
          heat.oNoff(false);
          exitInStep();
          Serial.println(F("Step 2 done"));
        }
        previousMillis11 = currentTime;
      }
    }
    break;
    //Шаг 3 вносим фермент и время t
  case 3: //таймер 20 минут
    if (!var)
    {
      lcd.setCursor(0, 1);
      lcd.print(F("add ferment"));
    }
    btContinue();
    if (var)
    {
      if (currentTime - previousMillis11 > 1000) // текущие - предыдущие
      {
        if (getTimeInMin() - start_time >= 1)
        {
          exitInStep();
          Serial.println(F("Step 3 done"));
        }
        previousMillis11 = currentTime;
      }
    }
    break;

  case 4: //Жднем нажатие кнопки для включение сервы на 30 секунд
    if (btContinue())
      start_time = getTimeInSec();

    if (var)
    {
      controlMotor(true);                        // нажали на кнопку включем движок
      if (currentTime - previousMillis11 > 1000) // текущие - предыдущие
      {
        if (getTimeInSec() - start_time >= 30)
        {
          controlMotor(false);
          exitInStep();
          Serial.println(F("Step 4 done"));
        }
        previousMillis11 = currentTime;
      }
    }
    break;

  case 5:
    //вывкести сообщение о смени насадки
    if (!var)
    {
      lcd.setCursor(0, 1);
      lcd.print(F("сhange nozzle"));
    }

    btContinue();

    if (var)
    {
      controlMotor(true);                       // нажали на кнопку включем движок
      heat.heatTo(&temp, setTemp1, &varHeatTo); //и нагреваем t

      if (currentTime - previousMillis11 > 1000)
      {
        if (getTimeInMin() - start_time >= 1)
        {
          controlMotor(false);
          buzzer(); //через 15 минут вырбуаем движок и пищим бузером
          exitInStep();
          Serial.println(F("Step 5 done"));
        }
        previousMillis11 = currentTime;
      }
    }
    break;

  // нагрев до T  и t держим температуру
  case 6:
    btContinue();
    if (var)
    {
      heat.tempMaint(&temp, setTemp1, 1); //греем до 50 и ждем 1.5 часа
      controlMotor(true);
      if (currentTime - previousMillis11 > 1000)
      {
        if (getTimeInMin() - start_time >= 2)
        {
          exitInStep();
          step = 1;
          menu = 0;
          Serial.println(F("Step 6 done"));
        }
        previousMillis11 = currentTime;
      }
    }
    break;
  }
}
void modeStop()
{
  heat.oNoff(false);
  controlMotor(false);
  digitalWrite(valve, LOW);
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(F("stop"));
}

void modePaster()
{
  btContinue();
  if (var)
  {

    heat.heatTo(&temp, setTemp1, &varHeatTo);
    if (!varHeatTo)
    {
      digitalWrite(valve, HIGH);
      if (temp <= 26)
      {
        digitalWrite(valve, LOW);
        //step = -1;
        var = false;
        varHeatTo = true;
        lcd.setCursor(6, 1); // по функционалу но вроде безопаснее
        lcd.print(F("done"));
        menu = 0;
        buzzer();
      }
    }
  }
}

void menuSwipe(uint8_t *menu)
{

  switch (*menu)
  {
  case 0:
    //step = -1;
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("stop"));
    break;

  case 1:
    step = 1;
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("cheese"));

    break;

  case 2:
    // step = 7;
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(F("pasteriza"));
    break;
  }
}