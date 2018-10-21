#include <ArduinoJson.h>
#include <Adafruit_BMP085.h>
#include <SimpleDHT.h>

Adafruit_BMP085 bmp;
SimpleDHT22 dht22;

void setup()
{ 
  Serial.begin(9600);                               //инициализируем серийный порт
  bmp.begin();                                      //инициализируем датчик температуры
}  
class sensor                //класс сенсор
{ byte hum;                 //влажность
  byte temp;                //температура
  float pres;               //давление
  float tempTotal;          //средняя температура
public:
  sensor();                 //конструктор по умолчанию
  void readData();          //функция для считывания данных
  void sendData();          //функция для отправки данных
};
sensor :: sensor()
{
  hum = 0;                  //устанавливаем значение 0 для всех показаний
  temp = 0;
  pres = 0;
  tempTotal = 0;
}
void sensor :: readData()
{
  int err = SimpleDHTErrSuccess;                                                  
  if ((err = dht22.read(9, &temp, &hum, NULL)) != SimpleDHTErrSuccess) return;  //считываем показания датчика dht22
  pres = bmp.readPressure()/133.3;                                              //считываем показания давления
  tempTotal += (int)bmp.readTemperature();
  tempTotal /= 2;                                                               //берем среднее значение температуры 
}
void sensor :: sendData()
{
    StaticJsonBuffer<200> jsonBuffer;                 //создаем буфер для формирования json 
    JsonObject& root = jsonBuffer.createObject();     //создаем объект json                 
    root["temp"] = tempTotal;                        //инициализируем поле объекта json значением температуры
    root["hum"] = (float) hum;                       //значением влажности
    root["bar"] = pres;                              //значением давления
    
    root.printTo(Serial);                            //отправляем данные в серийный порт
   
   jsonBuffer.clear();                              //очищаем jsonBuffer
}
sensor sens;                                  //объект класса сенсор
int incomingInfo = 0;
void loop()                                   //функция выполнения (выполняется зацикленно)
{ 
  sens.readData();                            //считываем показания с датчиков
  incomingInfo = Serial.read();               //считываем информацию по серийному порту
  if (incomingInfo == 49)                     //если получен код на отправку
    { 
      sens.sendData();                          //отправляем данные
    }
    incomingInfo = 0;
}

