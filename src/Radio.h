#include "ELECHOUSE_CC1101_SRC_DRV.h"
#include "CRC8.h"

class Radio
{
public:
  bool Initialize()
  {
    ELECHOUSE_cc1101.setGDO0(gdo);
    if (ELECHOUSE_cc1101.getCC1101())
    { // Check the CC1101 Spi connection.
      Serial.println("Radio connection OK");
      isReady = true;
    }
    else
    {
      Serial.println("Radio connection Error");
      return false;
    }
    // Инициализируем cc1101
    ELECHOUSE_cc1101.SpiStrobe(0x30); // reset
    ELECHOUSE_cc1101.SpiWriteBurstReg(0x00, rfSettings, 0x2F);
    ELECHOUSE_cc1101.SpiStrobe(0x33); // Calibrate frequency synthesizer and turn it off
    delay(1);
    ELECHOUSE_cc1101.SpiStrobe(0x3A); // Flush the RX FIFO buffer
    ELECHOUSE_cc1101.SpiStrobe(0x3B); // Flush the TX FIFO buffer
    ELECHOUSE_cc1101.SpiStrobe(0x34); // Enable RX
    return true;
  }

  // функция отправки пакета
  bool Send(byte *buff)
  {
    if (!isReady)
    {
      Serial.println("Radio is not ready.");
      return false;
    }
    ELECHOUSE_cc1101.SpiStrobe(0x33); // Calibrate frequency synthesizer and turn it off
    delay(1);
    ELECHOUSE_cc1101.SpiStrobe(0x3B);             // Flush the TX FIFO buffer
    ELECHOUSE_cc1101.SpiStrobe(0x36);             // Exit RX / TX, turn off frequency synthesizer and exit
    ELECHOUSE_cc1101.SpiWriteReg(0x3e, 0xC4);     // выставляем мощность 10dB
    ELECHOUSE_cc1101.SendData(buff, buff[0] + 1); // отправляем пакет
    ELECHOUSE_cc1101.SpiStrobe(0x3A);             // Flush the RX FIFO buffer
    ELECHOUSE_cc1101.SpiStrobe(0x34);             // Enable RX
    String s = "Packet sent: ";
    for (int i = 0; i < buff[0] + 1; i++)
    {
      s += String(buff[i], HEX) + " ";
    }
    Serial.println(s);
    return true;
  }

  // функция приёма пакета (помещает его в resultbuffer[])
  bool Receive(byte packetCount, byte *recBuffer)
  {
    if (!isReady)
    {
      Serial.println("Radio is not ready.");
      return false;
    }
    for (byte i = 0; i < 61; i++)
    {
      recBuffer[i] = 0x00;
    }
    tmr.start();
    byte PackCount = 0; // счётчик принятых из эфира пакетов
    byte bytecount = 0; // указатель байтов в результирующем буфере
    while (!tmr.tick() && PackCount != packetCount)
    {
      if (ELECHOUSE_cc1101.CheckReceiveFlag())
      {
        PackCount++;
        /*
              //Rssi Level in dBm
              Serial.print("Rssi: ");
              Serial.println(ELECHOUSE_cc1101.getRssi());

              //Link Quality Indicator
              Serial.print("LQI: ");
              Serial.println(ELECHOUSE_cc1101.getLqi());
            */

        // Get received Data and calculate length
        byte buffer[61] = {0};
        int len = ELECHOUSE_cc1101.ReceiveData(buffer);
        // подшиваем 1/3 пакета в общий пакет
        for (int i = 1; i < len; i++)
        {
          recBuffer[bytecount] = buffer[i];
          bytecount++;
        }
        ELECHOUSE_cc1101.SpiStrobe(0x36); // Exit RX / TX, turn off frequency synthesizer and exit
        ELECHOUSE_cc1101.SpiStrobe(0x3A); // Flush the RX FIFO buffer
        ELECHOUSE_cc1101.SpiStrobe(0x3B); // Flush the TX FIFO buffer
        // delay(1);
        ELECHOUSE_cc1101.SpiStrobe(0x34); // Enable RX
      }
    }
    if (PackCount == 0)
    {
      Serial.println("No packets received!");
      return false;
    }
    Serial.println("Packets received: " + String(PackCount));
    // Печатаем общий пакет
    String s;
    for (int i = 0; i < bytecount; i++)
    {
      s += String(recBuffer[i], HEX) + " ";
    }
    Serial.println(s);
    // Test CRC ------------------------------
    crc.reset();
    crc.setPolynome(crcPolynome);
    for (int i = 2; i < (bytecount - 2); i++)
    {
      crc.add(recBuffer[i]);
    }
    byte myCRC = crc.getCRC();
    byte receivedCrc = recBuffer[bytecount - 2];
    //-----------------------------------------
    if (receivedCrc == myCRC)
      Serial.println("CRC matches");
    else
      Serial.println("CRC does not match");
    return receivedCrc == myCRC;
  }

private:
  // Настройки для CC1101 с форума (47 бит)
  byte rfSettings[47] = {
      0x0D, // IOCFG2              GDO2 Output Pin Configuration
      0x2E, // IOCFG1              GDO1 Output Pin Configuration
      0x06, // IOCFG0              GDO0 Output Pin Configuration
      0x4F, // FIFOTHR             RX FIFO and TX FIFO Thresholds
      0xD3, // SYNC1               Sync Word, High Byte
      0x91, // SYNC0               Sync Word, Low Byte
      0x3C, // PKTLEN              Packet Length
      0x00, // PKTCTRL1            Packet Automation Control
      0x41, // PKTCTRL0            Packet Automation Control
      0x00, // ADDR                Device Address
      0x16, // CHANNR              Channel Number
      // 0x0B,  // CHANNR              Channel Number
      0x0F, // FSCTRL1             Frequency Synthesizer Control
      0x00, // FSCTRL0             Frequency Synthesizer Control
      0x10, // FREQ2               Frequency Control Word, High Byte
      0x8B, // FREQ1               Frequency Control Word, Middle Byte
      0x54, // FREQ0               Frequency Control Word, Low Byte
      0xD9, // MDMCFG4             Modem Configuration
      0x83, // MDMCFG3             Modem Configuration
      0x13, // MDMCFG2             Modem Configuration
      0xD2, // MDMCFG1             Modem Configuration
      0xAA, // MDMCFG0             Modem Configuration
      0x31, // DEVIATN             Modem Deviation Setting
      0x07, // MCSM2               Main Radio Control State Machine Configuration
      0x0C, // MCSM1               Main Radio Control State Machine Configuration
      0x08, // MCSM0               Main Radio Control State Machine Configuration
      0x16, // FOCCFG              Frequency Offset Compensation Configuration
      0x6C, // BSCFG               Bit Synchronization Configuration
      0x03, // AGCCTRL2            AGC Control
      0x40, // AGCCTRL1            AGC Control
      0x91, // AGCCTRL0            AGC Control
      0x87, // WOREVT1             High Byte Event0 Timeout
      0x6B, // WOREVT0             Low Byte Event0 Timeout
      0xF8, // WORCTRL             Wake On Radio Control
      0x56, // FREND1              Front End RX Configuration
      0x10, // FREND0              Front End TX Configuration
      0xE9, // FSCAL3              Frequency Synthesizer Calibration
      0x2A, // FSCAL2              Frequency Synthesizer Calibration
      0x00, // FSCAL1              Frequency Synthesizer Calibration
      0x1F, // FSCAL0              Frequency Synthesizer Calibration
      0x41, // RCCTRL1             RC Oscillator Configuration
      0x00, // RCCTRL0             RC Oscillator Configuration
      0x59, // FSTEST              Frequency Synthesizer Calibration Control
      0x59, // PTEST               Production Test
      0x3F, // AGCTEST             AGC Test
      0x81, // TEST2               Various Test Settings
      0x35, // TEST1               Various Test Settings
      // 0x0B,  // TEST0               Various Test Settings
      0x09 // TEST0               Various Test Settings
  };
  CRC8 crc;
  TimerMs tmr = TimerMs(2000, 0, 0);
  bool isReady = false;
};
