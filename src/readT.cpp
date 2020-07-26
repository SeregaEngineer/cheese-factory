
#include "readT.h"
bool flag = true;
OneWire ds(9);

void readTemp(float array[])
{
  byte data[4];
  //float temp;

  if (flag)
  {
    ds.reset();     // Начинаем взаимодействие со сброса всех предыдущих команд и параметров
    //ds.write(0xCC); // Даем датчику DS18b20 команду пропустить поиск по адресу. В нашем случае только одно устрйоство
    ds.skip();
    ds.write(0x44); // Даем датчику DS18b20 команду измерить температуру. Само значение температуры мы еще не получаем - датчик его положит во внутреннюю память
    //Serial.println(F("send"));
    flag = false;
  }

  else
  {
   ds.reset(); // Теперь готовимся получить значение измеренной температуры
    ds.select(Tm);
    //ds.write(0xCC);
    ds.write(0xBE);      // Просим передать нам значение регистров со значением температуры
    data[0] = ds.read(); // Читаем младший байт значения температуры
    data[1] = ds.read(); // А теперь старший}
    //Serial.println(F("read"));
    array[0] = ((data[1] << 8) | data[0]) * 0.0625;
   
    ds.reset();
    ds.select(Tw);
    //ds.write(0xCC);
    ds.write(0xBE);      // Просим передать нам значение регистров со значением температуры
    data[2] = ds.read(); // Читаем младший байт значения температуры
    data[3] = ds.read(); // А теперь старший}
    //Serial.println(F("read"));
    
    array[1] =((data[3] << 8) | data[2]) * 0.0625;
    flag = true;
  }
}
