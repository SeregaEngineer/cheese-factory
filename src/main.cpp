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
uint8_t Temp_paster = 27;     // Уставка пастеризации
uint8_t setTemp2 = 37;
uint32_t timeMin, timeSec;
//uint16_t setTimeMin;
uint16_t time_display;
bool run = false;       //Взводим когда идет работа что бы заблокирвоать меню
bool buz_status = true; //для единичного вклчюения буззера на стадии нагрева
uint32_t start_time;    // переменны для записи начанало поддержания температуры
bool var = false;       // Для  для включения сервы после нажатия на кнопку
bool varHeatTo = false; //перемннная для функции нагреть до
int8_t menu = 0;        // переменная для работы меню
Encoder enc(10, 9, 8);
LiquidCrystal_I2C lcd(0x3F, 16, 2);
Heater heat(heater);
uint8_t cheese[9];        // массив с настройками рецепт
int8_t brand_cheese = 10; //Переменная для выбора
uint8_t i = 0;
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

  if (enc.isRightH() && !run) // Добавить условия со step что если тема мутится не надо менять рецпты на ходу
  {
    menu++;
    if (menu > 2)
    {
      menu = 0;
    }
    menuSwipe(&menu);
  }

  if (enc.isLeftH() && !run)
  {
    menu--;
    if (menu < 0)
    {
      menu = 2;
    }
    menuSwipe(&menu);
  }
  //выбор типа сыра
  if (menu == 1 && !run && enc.isRight())
  {
    brand_cheese += 10;
    if (brand_cheese > 20)
    {
      brand_cheese = 10;
    }
    lcd.setCursor(8, 1);
    lcd.print(brand_cheese / 10);
    Serial.print(brand_cheese);
  }

  if (menu == 1 && !run && enc.isLeft())
  {
    brand_cheese -= 10;
    if (brand_cheese < 10)
    {
      brand_cheese = 20;
    }
    lcd.setCursor(8, 1);
    lcd.print(brand_cheese / 10);
    Serial.print(brand_cheese);
  }


   if (menu == 2 && !run && enc.isRight())
  {
    Temp_paster++;
    lcd.setCursor(11, 1);
    lcd.print(Temp_paster);
    
  }

  if (menu == 2 && !run && enc.isLeft())
  {
    Temp_paster--;
    lcd.setCursor(11, 1);
    lcd.print(Temp_paster);
  
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
bool btContinue(uint16_t time_process)
{
  if (!digitalRead(btn_on))
  {
    run = true;
    //start_time = getTimeInMin();
    start_time = now();
    var = true;
    time_display = time_process;
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
  heat.oNoff(false); // на выходе из каждго шага выключаем Тен, очень сомнительно
                     // lcd.setCursor(7, 1); // по функционалу но вроде безопаснее
  //lcd.print(F("done"));
}

// Режим приготовления сыра
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

    if (btContinue(NULL))
    {
      eeprom_read_block((void *)&cheese, brand_cheese, sizeof(cheese));
      Serial.print(var);
      varHeatTo = true;
      //step = 1; // ждем нажатия кнопки для запуска
    }
    if (var)
    { //Нагрев
      heat.heatTo(&temp, cheese[0], &varHeatTo);
      if (temp > cheese[0])
      {
        Serial.println(F("Step 1 done"));
        exitInStep();
      }
    }
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
    btContinue(cheese[1]);
    if (var)
    {
      heat.tempMaint(&temp, cheese[2], 1);
      //Сообщение о Поддержание температуры или номер шага
      if (currentTime - previousMillis11 > 1000) // текущие - предыдущие
      {
        timerMin();
        //каждую сек обеновляем диспей и пишем скольок времени осталось
        if (now() - start_time >= cheese[1] * 60)
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
    btContinue(cheese[3]);
    if (var)
    {
      if (currentTime - previousMillis11 > 1000) // текущие - предыдущие
      {
        timerMin();
        if (now() - start_time >= cheese[3] * 60)
        {
          exitInStep();
          Serial.println(F("Step 3 done"));
        }
        previousMillis11 = currentTime;
      }
    }
    break;

  case 4: //Жднем нажатие кнопки для включение сервы на 30 секунд
    if (!var)
    {
      lcd.setCursor(0, 1);
      lcd.print(F("Check sg"));
    }
    btContinue(cheese[4]);

    if (var)
    {
      controlMotor(true);                        // нажали на кнопку включем движок
      if (currentTime - previousMillis11 > 1000) // текущие - предыдущие
      {
        if (now() - start_time >= cheese[4])
        {
          //timerMin();
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

    btContinue(cheese[5]);

    if (var)
    {
      controlMotor(true);                        // нажали на кнопку включем движок
      heat.heatTo(&temp, cheese[6], &varHeatTo); //и нагреваем t

      if (currentTime - previousMillis11 > 1000)
      {
        timerMin();
        if (now() - start_time >= cheese[5] * 60)
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
    if (!var)
    {
      lcd.setCursor(0, 1);
      lcd.print(F("need help"));
    }

    btContinue(cheese[8]);
    if (var)
    {
      heat.tempMaint(&temp, cheese[7], 1); //греем до 50 и ждем 1.5 часа
      controlMotor(true);
      if (currentTime - previousMillis11 > 1000)
      {
        timerMin();
        if (now() - start_time >= cheese[8] * 60)
        {
          exitInStep();
          step = 1;
          menu = 0;
          menuSwipe(&menu);
          Serial.println(F("Step 6 done"));
        }
        previousMillis11 = currentTime;
      }
    }
    break;
  }
}

//Режим стоп
void modeStop()
{

  heat.oNoff(false);
  controlMotor(false);
  digitalWrite(valve, LOW);
  run = false;
}
//Режим пастеризации
void modePaster()
{

  btContinue(NULL); //////!!!!!!!!!!!!!!
  if (var)
  {

    heat.heatTo(&temp, Temp_paster, &varHeatTo);
    if (!varHeatTo)
    {
      digitalWrite(valve, HIGH);
      if (temp <= 26)
      {
        digitalWrite(valve, LOW);
        //step = -1;
        var = false;
        varHeatTo = true;
        lcd.clear();
        lcd.setCursor(0, 1); // по функционалу но вроде безопаснее
        lcd.print(F("stop"));
        menu = 0;
        buzzer();
      }
    }
  }
}

void menuSwipe(int8_t *menu)
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
    lcd.setCursor(11, 1);
    lcd.print(Temp_paster);
    break;
  }
}

void timerMin()
{

  i++;
  lcd.setCursor(14, 0);
  lcd.print(" ");
  lcd.setCursor(15, 0);
  lcd.print(time_display);
  if (i == 60)
  {
    time_display--;
    // lcd.setCursor(14, 0);
    //lcd.print(" ");
    // lcd.setCursor(15, 0);
    // lcd.print(time_display);
    i = 0;
  }
}
