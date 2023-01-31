#include <Arduino.h>
#include "CRC8.h"

class Data5
{
public:
  float Sum;
  float T1;
  float T2;
  unsigned long Updated;

  void Clear()
  {
    T1 = 0;
    T2 = 0;
    Sum = 0;
    Updated = 0;
  }

  int FillRequestBuffer(byte *buff, uint16_t meterAddressValue)
  {
    buff[0] = 0x10; // длина пакета 16 байт
    buff[1] = 0x73;
    buff[2] = 0x55;                            // начало payload
    buff[3] = 0x21;                            // тип запроса
    buff[4] = 0x00;                            //
    buff[5] = meterAddressValue & 0xff;        // младший байт адреса счётчика
    buff[6] = (meterAddressValue >> 8) & 0xff; // старший байт адреса счётчика
    buff[7] = 0xff;                            // client id
    buff[8] = 0xff;                            // client id
    buff[9] = 0x05;                            //
    buff[10] = 0x00;                           // PIN
    buff[11] = 0x00;                           // PIN
    buff[12] = 0x00;                           // PIN
    buff[13] = 0x00;                           // PIN
    buff[14] = 0x00;
    // вычисляем и добавляем байт crc
    crc.restart();
    crc.setPolynome(crcPolynome);
    for (int i = 3; i < (buff[0] - 1); i++)
    {
      crc.add(buff[i]);
    }
    buff[15] = crc.getCRC(); // CRC
    buff[16] = 0x55;         // конец payload

    return 4;
  }

  bool Parse(byte *buff, uint16_t meterAddressValue)
  {
    if ((buff[0] == 0x73) and (buff[1] == 0x55) and (buff[2] == 0x1E) and (buff[6] == ((meterAddressValue)&0xff)) and (buff[7] == (((meterAddressValue) >> 8) & 0xff)) and (buff[8] == 0x5) and (buff[17] == 0x1) and (buff[44] == 0x55))
    {
      Sum = float((buff[25] << 16) | (buff[24] << 8) | buff[23]) / 100;
      T1 = float((buff[29] << 16) | (buff[28] << 8) | buff[27]) / 100;
      T2 = float((buff[33] << 16) | (buff[32] << 8) | buff[31]) / 100;
      Serial.println("SUM: " + String(Sum));
      Serial.println("T1: " + String(T1));
      Serial.println("T2: " + String(T2));
      Updated = millis();
      return true;
    }
    else
    {
      return false;
    }
  }

private:
  CRC8 crc;
};
