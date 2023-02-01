#include "Radio.h"

class MeterHandler
{
public:
    void Setup(Data5 &data5, Data9 &data9, MirtekGWConfig &config)
    {
        _data5 = &data5;
        _data9 = &data9;
        _config = &config;
        initRadio();
        _tmrMeter.start();
    }

    void Loop()
    {
        if (_config->Meter.MeterAddress == 0)
            return;
        if (_tmrMeter.tick())
        {
            Serial.println("meter handler timer tick");
            ProcessPacket59();
        }
    }

    bool ProcessPacket5()
    {
        return ProcessRequest(true, false);
    }

    bool ProcessPacket9()
    {
        return ProcessRequest(false, true);
    }

    bool ProcessPacket59()
    {
        return ProcessRequest(true, true);
    }

    void AttachData5Callback(void (*callback)())
    {
        _OnData5Callback = callback;
    }

    void AttachData9Callback(void (*callback)())
    {
        _OnData9Callback = callback;
    }

private:
    Data5 *_data5;
    Data9 *_data9;
    MirtekGWConfig *_config;
    Radio radio;
    unsigned int _failedCount = 0;
    unsigned int _maxFailedCount = 10;
    unsigned long _sleepTo = 0;
    TimerMs _tmrMeter = TimerMs(tmr_mqtt_time_min * 60 * 1000, 0, 0);
    std::function<void(void)> _OnData5Callback;
    std::function<void(void)> _OnData9Callback;

    bool initRadio()
    {
        if (_config->Meter.MeterAddress == 0)
        {
            return false;
        }
        return radio.Initialize();
    }

    bool ProcessRequest(bool p5, bool p9)
    {
        if (!p5 && !p9)
            return false;
        if (millis() < _sleepTo)
        {
            Serial.println("Processing skipped due to errors");
            return false;
        }
        bool result = (p5 && ProcessPacket5Inner() || !p5) && (p9 && ProcessPacket9Inner() || !p9);
        if (!result)
        {
            _failedCount++;
            Serial.print("Meter processing failed, total number of consecutive failures is ");
            Serial.println(_failedCount);
        }
        else
        {
            _sleepTo = 0;
            _failedCount = 0;
        }
        if (_failedCount > _maxFailedCount)
        {
            Serial.println("Max number of errors reached, pause processing.");
            _failedCount = 0;
            _sleepTo = millis() + tmr_mqtt_time_min * 60 * 1000 /* _maxFailedCount*/;
        }
        return result;
    }

    bool ProcessPacket5Inner()
    {
        if (_mirtek_radio_test == 1)
        {
            _data5->Updated = millis();
            _data5->Sum += 100;
            _data5->T1 += 10;
            _data5->T2 += 5;
            if (_OnData5Callback)
                _OnData5Callback();
            return true;
        }
        if (_config->Meter.MeterAddress == 0)
            return false;
        byte buff[61] = {0};
        int packetCount = _data5->FillRequestBuffer(buff, _config->Meter.MeterAddress);
        if (radio.Send(buff) && radio.Receive(packetCount, buff))
        {
            if (_data5->Parse(buff, _config->Meter.MeterAddress))
            {
                if (_OnData5Callback)
                    _OnData5Callback();
                return true;
            }
            else
            {
                Serial.println("PARSE ERROR 5");
            }
        }
        else
        {
            Serial.println("CRC ERROR 5");
        }
        return false;
    }

    bool ProcessPacket9Inner()
    {
        if (_mirtek_radio_test == 1)
        {
            _data9->Updated = millis();
            _data9->I1 += 10;
            _data9->I2 += 9;
            _data9->I3 += 8;
            _data9->V1 += 7;
            _data9->V2 += 6;
            _data9->V3 += 5;
            _data9->Freq += 50;
            _data9->Cos += 1;
            if (_OnData9Callback)
                _OnData9Callback();
            return true;
        }
        if (_config->Meter.MeterAddress == 0)
            return false;
        byte buff[61] = {0};
        int packetCount = _data9->FillRequestBuffer(buff, _config->Meter.MeterAddress);
        if (radio.Send(buff) && radio.Receive(packetCount, buff))
        {
            if (_data9->Parse(buff, _config->Meter.MeterAddress))
            {
                if (_OnData9Callback)
                    _OnData9Callback();
                return true;
            }
            else
            {
                Serial.println("PARSE ERROR 9");
            }
        }
        else
        {
            Serial.println("CRC ERROR 9");
        }
        return false;
    }
};