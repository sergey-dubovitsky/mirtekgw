#include <Arduino.h>
#include "CRC8.h"

class Data9
{
public:
  float Freq;
  float Cos;
  float V1;
  float V2;
  float V3;
  float I1;
  float I2;
  float I3;
  unsigned long Updated;

  void Clear()
  {
    Freq = 0;
    Cos = 0;
    V1 = 0;
    V2 = 0;
    V3 = 0;
    I1 = 0;
    I2 = 0;
    I3 = 0;
    Updated = 0;
  }

  int FillPreRequestBuffer(byte *buff, uint16_t meterAddressValue)
  {
    buff[0] = 0x10; // длина пакета 16 байт
    buff[1] = 0x73;
    buff[2] = 0x55;                                // начало payload
    buff[3] = 0x20;                                // тип запроса
    buff[4] = 0x00;                                //
    buff[5] = ((meterAddressValue)) & 0xff;        // младший байт адреса счётчика
    buff[6] = (((meterAddressValue)) >> 8) & 0xff; // старший байт адреса счётчика
    buff[7] = 0xff;                                //
    buff[8] = 0xff;                                //
    buff[9] = 0x1C;                                //
    buff[10] = 0x00;                               // PIN
    buff[11] = 0x00;                               // PIN
    buff[12] = 0x00;                               // PIN
    buff[13] = 0x00;                               // PIN
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

  int FillRequestBuffer(byte *buff, uint16_t meterAddressValue)
  {
    buff[0] = 0x10; // длина пакета 16 байт
    buff[1] = 0x73;
    buff[2] = 0x55;                                // начало payload
    buff[3] = 0x21;                                // тип запроса
    buff[4] = 0x00;                                //
    buff[5] = ((meterAddressValue)) & 0xff;        // младший байт адреса счётчика
    buff[6] = (((meterAddressValue)) >> 8) & 0xff; // старший байт адреса счётчика
    buff[7] = 0xff;                                // client id
    buff[8] = 0xff;                                // client id
    buff[9] = 0x2b;                                //
    buff[10] = 0x00;                               // PIN
    buff[11] = 0x00;                               // PIN
    buff[12] = 0x00;                               // PIN
    buff[13] = 0x00;                               // PIN
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
    if ((buff[0] == 0x73) and (buff[1] == 0x55) and (buff[2] == 0x1E) and (buff[6] == ((meterAddressValue)&0xff)) and (buff[7] == (((meterAddressValue) >> 8) & 0xff)) and (buff[8] == 0x2B) and (buff[12] == 0x0))
    {
      Freq = float((buff[24] | (buff[25] << 8))) / 100;
      Cos = float((buff[26] | (buff[27] << 8))) /*/ 1000*/ / 10;
      V1 = float((buff[28] | (buff[29] << 8))) / 100;
      V2 = float((buff[30] | (buff[31] << 8))) / 100;
      V3 = float((buff[32] | (buff[33] << 8))) / 100;
      I1 = float(buff[34] | (buff[35] << 8) | (buff[36] << 16)) / 1000;
      I2 = float(buff[37] | (buff[38] << 8) | (buff[39] << 16)) / 1000;
      I3 = float(buff[40] | (buff[41] << 8) | (buff[42] << 16)) / 1000;
      Serial.println("Freq: " + String(Freq));
      Serial.println("Cos: " + String(Cos));
      Serial.println("V1: " + String(V1));
      Serial.println("V2: " + String(V2));
      Serial.println("V3: " + String(V3));
      Serial.println("I1: " + String(I1));
      Serial.println("I2: " + String(I2));
      Serial.println("I3: " + String(I3));
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
