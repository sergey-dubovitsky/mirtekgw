// https://github.com/gysmo38/mitsubishi2MQTT/tree/f000d6e82cefd80347276dc4edf2d6ea201b3902
#include <Arduino.h>
#include <FS.h>
#include <WiFi.h>
// #include <WiFiUdp.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <TimerMs.h>
#include <ArduinoOTA.h>
// https://github.com/knolleary/pubsubclient
#include <PubSubClient.h>

#include "main.h"
#include "Const.h"
#include "Data5.h"
#include "Data9.h"
#include "ConfigLoader.h"
#include "WebHandler.h"
#include "MeterHandler.h"
#include "prometheus.h"
#include "mqtt_templates.h"

WebServer server(80);
WiFiClient espClient;
DNSServer dnsServer;
PubSubClient mqttClient;

MirtekGWConfig config;
TimerMs tmrUptime(60 * 1000, 0, 0);
WebHandler webHandler;
MeterHandler meterHandler;
unsigned long wifi_timeout = 0;
unsigned long mqtt_timeout = 0;
bool captive = false;
int uploadError = 0;

Data5 data5;
Data9 data9;

const PROGMEM uint32_t WIFI_RETRY_INTERVAL_MS = 5 * 60 * 1000;
const byte DNS_PORT = 53;
const PROGMEM uint32_t MQTT_RETRY_INTERVAL_MS = 30 * 1000;
const PROGMEM char *mqtt_payload_available = "online";
const PROGMEM char *mqtt_payload_unavailable = "offline";

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  webHandler.Set(server, config);
  webHandler.AttachData(data5, data9);
  if (!SPIFFS.begin())
  {
    Serial.println(F("SPIFFS not initialized, formatting."));
    SPIFFS.format();
    if (!SPIFFS.begin())
    {
      Serial.println(F("SPIFFS initialization error, restart."));
      ESP.restart();
    }
  }
  else
  {
    Serial.println(F("SPIFFS initialized."));
  }

  config.Load();

  if (initWifi())
  {
    webHandler.StartNormalMode();
    webHandler.AttachMetricsCallback(getMetrics);
    webHandler.AttachResetCallback(OnResetCallback);
    webHandler.AttachRebootCallback(OnRebootCallback);
    webHandler.AttachMeterSaveCallback(OnMeterSaveCallback);
    webHandler.AttachMqttSaveCallback(OnMqttSaveCallback);
    webHandler.AttachWifiSaveCallback(OnWifiSaveCallback);
    webHandler.AttachUploadGetStatusCallback(OnUploadGetStatusCallback);
    webHandler.AttachUploadRebootCallback(OnUploadRebootCallback);
    webHandler.AttachUploadLoopCallback(OnUploadLoopCallback);
    webHandler.AttachUpgradeStartCallback(OnUpgradeStartCallback);
    webHandler.AttachRequestCallback(OnRequestData5Callback, OnRequestData9Callback);
    initMqtt();
  }
  else
  {
    dnsServer.start(DNS_PORT, "*", config.Wifi.IpAddress);
    captive = true;
    webHandler.StartCaptiveMode(OnCaptiveSaveCallback, OnCaptiveRebootCallback);
  }
  setupStorage();
  incrementBootCounter();

  tmrUptime.start();

  // это нужно для вывода в serial информации об OTA-обновлении
  ArduinoOTA
      .onStart([]()
               {
       String type;
       if (ArduinoOTA.getCommand() == U_FLASH)
         type = "sketch";
       else // U_SPIFFS
         type = "filesystem";

       // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
       Serial.println("Start updating " + type); })
      .onEnd([]()
             { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
       Serial.printf("Error[%u]: ", error);
       if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
       else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
       else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
       else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
       else if (error == OTA_END_ERROR) Serial.println("End Failed"); });
  ArduinoOTA.begin();
  data5.Clear();
  data9.Clear();
  meterHandler.Setup(data5, data9, config);
  meterHandler.AttachData5Callback(OnMeterData5Callback);
  meterHandler.AttachData9Callback(OnMeterData9Callback);
  Serial.println(F("\r\n\r\n\r\n\r\nSETUP END\r\n\r\n\r\n\r\n"));
}

void loop()
{
  server.handleClient();
  meterHandler.Loop();
  // этим двум loop тут не место, но если внести в if ниже, то не работает переподключение к wifi
  loopWiFiConnectivity();
  loopMqttConnect();
  if (!captive)
  {

    if (tmrUptime.tick())
    {
      tmrUptimeCallback();
    }
    meterHandler.Loop();
  }
  else
  {
    dnsServer.processNextRequest();
  }
  ArduinoOTA.handle();
}

boolean initWifi()
{
  WiFi.setHostname(config.Wifi.Hostname.c_str());
  if (config.Wifi.SSID[0] != '\0')
  {
    if (connectWifi())
    {
      return true;
    }
  }
  Serial.println(F("Starting in AP mode"));
  WiFi.mode(WIFI_AP);
  WiFi.persistent(false);
  WiFi.softAPConfig(config.Wifi.IpAddress, config.Wifi.IpAddress, config.Wifi.NetworkMask);
  WiFi.softAP(config.Wifi.Hostname.c_str(), "");
  delay(3000);
  return false;
}

bool connectWifi()
{
  Serial.println(config.Wifi.Hostname);
  WiFi.setHostname(config.Wifi.Hostname.c_str());
  if (WiFi.getMode() != WIFI_STA)
  {
    WiFi.mode(WIFI_STA);
    delay(10);
  }
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.begin(config.Wifi.SSID.c_str(), config.Wifi.Password.c_str());
  Serial.print(F("Connecting to "));
  Serial.println(config.Wifi.SSID);
  unsigned long wifi_timeout = millis() + 30000;
  while (WiFi.status() != WL_CONNECTED && millis() < wifi_timeout)
  {
    delay(1);
    Serial.write('.');
  }
  Serial.println();
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println(F("Failed to connect to wifi"));
    return false;
  }
  Serial.print(F("Connected to "));
  Serial.println(config.Wifi.SSID);
  while (WiFi.localIP().toString() == "0.0.0.0" || WiFi.localIP().toString() == "")
  {
    Serial.write('.');
    delay(500);
  }
  if (WiFi.localIP().toString() == "0.0.0.0" || WiFi.localIP().toString() == "")
  {
    Serial.println(F("Failed to get IP address"));
    return false;
  }
  Serial.print(F("IP address: "));
  Serial.println(WiFi.localIP());
  return true;
}

bool initMqtt()
{
  if (!config.Mqtt.Loaded)
  {
    return false;
  }
  mqttClient.setBufferSize(1024);
  mqttClient.setClient(espClient);
  mqttClient.setServer(config.Mqtt.Server.c_str(), atoi(config.Mqtt.Port.c_str()));
  loopMqttConnect();

  return config.Mqtt.Loaded;
}

void loopWiFiConnectivity()
{
  // отвалились от wifi - рестарт
  if (WiFi.getMode() == WIFI_STA && WiFi.status() != WL_CONNECTED)
  {
    Serial.println(F("Restart due to wifi problems"));
    delay(1);
    ESP.restart();
  }
  // не смогли подключиться к сети из настроек - остаёмся в режиме точки и планируем ребут через 5 минут (= WIFI_RETRY_INTERVAL_MS)
  if (config.Wifi.Loaded && WiFi.getMode() == WIFI_AP && wifi_timeout == 0)
  {
    Serial.println(F("Plan to restart due to wifi problems"));
    wifi_timeout = millis() + WIFI_RETRY_INTERVAL_MS;
  }
  // ребут из режима точки при наличии настроек wifi
  if (millis() > wifi_timeout && wifi_timeout > 0)
  {
    Serial.println(F("Restart due to wifi timer"));
    delay(1);
    ESP.restart();
  }
}

void loopMqttConnect()
{
  // если не подключены к wifi то нечего и пробовать
  if (WiFi.getMode() != WIFI_STA || WiFi.status() != WL_CONNECTED)
  {
    return;
  }
  // не подключены?
  if (!mqttClient.connected())
  {
    // таймер еще не установлен ИЛИ время истекло?
    if (mqtt_timeout == 0 || millis() > mqtt_timeout)
    {
      mqtt_timeout = millis() + MQTT_RETRY_INTERVAL_MS;
      Serial.println(config.Mqtt.AvailabilityTopic);
      if (mqttClient.connect(config.Mqtt.FriendlyName.c_str(),
                             config.Mqtt.Username.c_str(),
                             config.Mqtt.Password.c_str(),
                             config.Mqtt.AvailabilityTopic.c_str(),
                             1,
                             true,
                             mqtt_payload_unavailable))
      {
        Serial.println(F("MQTT connected"));
        mqttClient.publish(config.Mqtt.AvailabilityTopic.c_str(), mqtt_payload_available, true);
        mqttClient.subscribe(config.Mqtt.CommandRebootTopic.c_str());
        mqttClient.subscribe(config.Mqtt.CommandRequestDataTopic.c_str());
        mqttClient.setCallback(mqttSubscribe);
        initHADiscovery();
        mqtt_timeout = 0;
      }
    }
  }
  mqttClient.loop();
}

void mqttSubscribe(char *topic, byte *payload, unsigned int length)
{
  // Copy payload into message buffer
  char message[length + 1];
  for (unsigned int i = 0; i < length; i++)
    message[i] = (char)payload[i];
  message[length] = '\0';
  String messageUpper = (String)message;
  messageUpper.toUpperCase();
  if (strcmp(topic, config.Mqtt.CommandRebootTopic.c_str()) == 0)
  {
    if (messageUpper != "")
    {
      Serial.println("MQTT reboot");
      delay(500);
      ESP.restart();
    }
  }
  else if (strcmp(topic, config.Mqtt.CommandRequestDataTopic.c_str()) == 0)
  {
    if (messageUpper != "")
    {
      meterHandler.ProcessPacket59();
    }
  }
}

void tmrUptimeCallback()
{
  Serial.println("tmrUptimeCallback");
  if (!!mqttClient.connected())
  {
    mqttClient.publish(config.Mqtt.UptimeTopic.c_str(), String(millis()).c_str());
  }
}

void OnCaptiveSaveCallback()
{
  config.Save();
  delay(500);
  ESP.restart();
}

void OnCaptiveRebootCallback()
{
  delay(500);
  ESP.restart();
}

void OnResetCallback()
{
  SPIFFS.format();
  delay(500);
  ESP.restart();
}

void OnRebootCallback()
{
  delay(500);
  ESP.restart();
}

void OnMeterSaveCallback()
{
  config.SaveMeter();
  delay(500);
  ESP.restart();
}

void OnMqttSaveCallback()
{
  config.SaveMqtt();
  delay(500);
  ESP.restart();
}

void OnWifiSaveCallback()
{
  config.SaveWifi();
  delay(500);
  ESP.restart();
}

int OnUploadGetStatusCallback()
{
  return uploadError;
}

void OnUploadRebootCallback()
{
  delay(500);
  ESP.restart();
}

void OnUploadLoopCallback()
{
  if (uploadError)
  {
    Update.end();
    return;
  }
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START)
  {
    if (upload.filename.c_str()[0] == 0)
    {
      uploadError = 1;
      return;
    }
    // save cpu by disconnect/stop retry mqtt server
    if (mqttClient.state() == MQTT_CONNECTED)
    {
      mqtt_timeout = millis() + MQTT_RETRY_INTERVAL_MS;
      mqttClient.disconnect();
    }
    uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
    if (!Update.begin(maxSketchSpace))
    { // start with max available size
      // Update.printError(Serial);
      uploadError = 2;
      return;
    }
  }
  else if (!uploadError && (upload.status == UPLOAD_FILE_WRITE))
  {
    if (upload.totalSize == 0)
    {
      if (upload.buf[0] != 0xE9)
      {
        // Serial.println(PSTR("Upload: File magic header does not start with 0xE9"));
        uploadError = 3;
        return;
      }
      uint32_t bin_flash_size = ESP.magicFlashChipSize((upload.buf[3] & 0xf0) >> 4);
#ifdef ESP32
      if (bin_flash_size > ESP.getFlashChipSize())
      {
#else
      if (bin_flash_size > ESP.getFlashChipRealSize())
      {
#endif
        // Serial.printl(PSTR("Upload: File flash size is larger than device flash size"));
        uploadError = 4;
        return;
      }
      if (ESP.getFlashChipMode() == 3)
      {
        upload.buf[2] = 3; // DOUT - ESP8285
      }
      else
      {
        upload.buf[2] = 2; // DIO - ESP8266
      }
    }
    if (!uploadError && (Update.write(upload.buf, upload.currentSize) != upload.currentSize))
    {
      // Update.printError(Serial);
      uploadError = 5;
      return;
    }
  }
  else if (!uploadError && (upload.status == UPLOAD_FILE_END))
  {
    if (Update.end(true))
    { // true to set the size to the current progress
      // snprintf_P(log, sizeof(log), PSTR("Upload: Successful %u bytes. Restarting"), upload.totalSize);
      // Serial.printl(log)
    }
    else
    {
      // Update.printError(Serial);
      uploadError = 6;
      return;
    }
  }
  else if (upload.status == UPLOAD_FILE_ABORTED)
  {
    // Serial.println(PSTR("Upload: Update was aborted"));
    uploadError = 7;
    Update.end();
  }
  delay(0);
}

void OnUpgradeStartCallback()
{
  uploadError = 0;
}

void OnMeterData5Callback()
{
  if (!!mqttClient.connected())
  {
    const size_t capacity = JSON_OBJECT_SIZE(3);
    DynamicJsonDocument doc(capacity);
    doc["SUM"] = ((int)(100.0 * data5.Sum)) / 100.0;
    doc["T1"] = ((int)(100.0 * data5.T1)) / 100.0;
    doc["T2"] = ((int)(100.0 * data5.T2)) / 100.0;
    String result;
    serializeJson(doc, result);
    mqttClient.publish(config.Mqtt.ConsumptionTopic.c_str(), result.c_str(), true);
  }
}

void OnMeterData9Callback()
{
  if (!!mqttClient.connected())
  {
    const size_t capacity = JSON_OBJECT_SIZE(8);
    DynamicJsonDocument doc(capacity);
    doc["Freq"] = ((int)(100.0 * data9.Freq)) / 100.0;
    doc["Cos"] = ((int)(100.0 * data9.Cos)) / 100.0;
    doc["I1"] = ((int)(100.0 * data9.I1)) / 100.0;
    doc["I2"] = ((int)(100.0 * data9.I2)) / 100.0;
    doc["I3"] = ((int)(100.0 * data9.I3)) / 100.0;
    doc["V1"] = ((int)(100.0 * data9.V1)) / 100.0;
    doc["V2"] = ((int)(100.0 * data9.V2)) / 100.0;
    doc["V3"] = ((int)(100.0 * data9.V3)) / 100.0;
    String result;
    serializeJson(doc, result);
    mqttClient.publish(config.Mqtt.MetricsTopic.c_str(), result.c_str(), true);
  }
}

void OnRequestData5Callback()
{
  meterHandler.ProcessPacket5();
}

void OnRequestData9Callback()
{
  meterHandler.ProcessPacket9();
}

void initHADiscovery()
{
  /*
  https://www.home-assistant.io/integrations/template/
  https://www.home-assistant.io/integrations/sensor.mqtt/#model
  https://www.home-assistant.io/integrations/mqtt/
  */
  String devId = config.DefaultHostname();
  String deviceInfo = FPSTR(mqtt_device_template);
  deviceInfo.replace(F("_DEV_URL_"), "http://" + WiFi.localIP().toString());
  deviceInfo.replace(F("_DEV_ID_"), devId);
  deviceInfo.replace(F("_DEV_NAME_"), config.Mqtt.FriendlyName);
  deviceInfo.replace(F("_DEV_SW_VER_"), _mirtekgw2_version);
  String availabilityBlock = FPSTR(mqtt_availability_payload);
  availabilityBlock.replace(F("_AV_TOPIC_"), config.Mqtt.AvailabilityTopic);
  availabilityBlock.replace(F("_AV_ON_"), FPSTR(mqtt_payload_available));
  availabilityBlock.replace(F("_AV_OFF_"), FPSTR(mqtt_payload_unavailable));

  String buttonReset = FPSTR(mqtt_button_config_template);
  buttonReset.replace(F("_DEV_JSON_"), deviceInfo);
  buttonReset.replace(F("_AVAILABILITY_"), availabilityBlock);
  buttonReset.replace(F("_PAYLOAD_"), F("reboot"));
  buttonReset.replace(F("_CMD_TOPIC_"), config.Mqtt.CommandRebootTopic);
  buttonReset.replace(F("_UNIQ_ID_"), devId + "_btn_reboot");
  buttonReset.replace(F("_OBJ_ID_"), devId + "_btn_reboot");
  buttonReset.replace(F("_TEXT_"), F("Перезагрузка"));
  mqttClient.publish(("homeassistant/button/btn_reboot/" + devId + "/config").c_str(), buttonReset.c_str());

  String buttonRequest = FPSTR(mqtt_button_template);
  buttonRequest.replace(F("_DEV_JSON_"), deviceInfo);
  buttonRequest.replace(F("_AVAILABILITY_"), availabilityBlock);
  buttonRequest.replace(F("_PAYLOAD_"), F("request"));
  buttonRequest.replace(F("_CMD_TOPIC_"), config.Mqtt.CommandRequestDataTopic);
  buttonRequest.replace(F("_UNIQ_ID_"), devId + "_btn_request");
  buttonRequest.replace(F("_OBJ_ID_"), devId + "_btn_request");
  buttonRequest.replace(F("_TEXT_"), F("Запросить данные"));
  mqttClient.publish(("homeassistant/button/btn_request/" + devId + "/config").c_str(), buttonRequest.c_str());

  String sumSensor = FPSTR(mqtt_sensor_template);
  sumSensor.replace(F("_DEV_JSON_"), deviceInfo);
  sumSensor.replace(F("_AVAILABILITY_"), availabilityBlock);
  sumSensor.replace(F("_UNIT_"), "kWh");
  sumSensor.replace(F("_DEV_CLA_"), "energy");
  sumSensor.replace(F("_STATE_TOPIC_"), config.Mqtt.ConsumptionTopic);
  sumSensor.replace(F("_VAL_TEMPL_"), F("{{value_json.SUM}}"));
  sumSensor.replace(F("_UNIQ_ID_"), devId + "_sensor_sum");
  sumSensor.replace(F("_NAME_"), "Sum");
  sumSensor.replace(F("_OBJ_ID_"), devId + "_sensor_sum");
  mqttClient.publish(("homeassistant/sensor/sensor_sum/" + devId + "/config").c_str(), sumSensor.c_str());

  String t1Sensor = FPSTR(mqtt_sensor_template);
  t1Sensor.replace(F("_DEV_JSON_"), deviceInfo);
  t1Sensor.replace(F("_AVAILABILITY_"), availabilityBlock);
  t1Sensor.replace(F("_UNIT_"), "kWh");
  t1Sensor.replace(F("_DEV_CLA_"), "energy");
  t1Sensor.replace(F("_STATE_TOPIC_"), config.Mqtt.ConsumptionTopic);
  t1Sensor.replace(F("_VAL_TEMPL_"), F("{{value_json.T1}}"));
  t1Sensor.replace(F("_UNIQ_ID_"), devId + "_sensor_t1");
  t1Sensor.replace(F("_NAME_"), "T1");
  t1Sensor.replace(F("_OBJ_ID_"), devId + "_sensor_t1");
  mqttClient.publish(("homeassistant/sensor/sensor_t1/" + devId + "/config").c_str(), t1Sensor.c_str());

  String t2Sensor = FPSTR(mqtt_sensor_template);
  t2Sensor.replace(F("_DEV_JSON_"), deviceInfo);
  t2Sensor.replace(F("_AVAILABILITY_"), availabilityBlock);
  t2Sensor.replace(F("_UNIT_"), "kWh");
  t2Sensor.replace(F("_DEV_CLA_"), "energy");
  t2Sensor.replace(F("_STATE_TOPIC_"), config.Mqtt.ConsumptionTopic);
  t2Sensor.replace(F("_VAL_TEMPL_"), F("{{value_json.T2}}"));
  t2Sensor.replace(F("_UNIQ_ID_"), devId + "_sensor_t2");
  t2Sensor.replace(F("_NAME_"), "T2");
  t2Sensor.replace(F("_OBJ_ID_"), devId + "_sensor_t2");
  mqttClient.publish(("homeassistant/sensor/sensor_t2/" + devId + "/config").c_str(), t2Sensor.c_str());

  String cSensor = FPSTR(mqtt_sensor_template);
  cSensor.replace(F("_DEV_JSON_"), deviceInfo);
  cSensor.replace(F("_AVAILABILITY_"), availabilityBlock);
  cSensor.replace(F("_UNIT_"), "%");
  cSensor.replace(F("_DEV_CLA_"), "power_factor");
  cSensor.replace(F("_STATE_TOPIC_"), config.Mqtt.MetricsTopic);
  cSensor.replace(F("_VAL_TEMPL_"), ("{{value_json.Cos}}"));
  cSensor.replace(F("_UNIQ_ID_"), devId + "_sensor_cos");
  cSensor.replace(F("_NAME_"), "Cos");
  cSensor.replace(F("_OBJ_ID_"), devId + "_sensor_cos");
  mqttClient.publish(("homeassistant/sensor/sensor_cos/" + devId + "/config").c_str(), cSensor.c_str());

  String fSensor = FPSTR(mqtt_sensor_template);
  fSensor.replace(F("_DEV_JSON_"), deviceInfo);
  fSensor.replace(F("_AVAILABILITY_"), availabilityBlock);
  fSensor.replace(F("_UNIT_"), "Hz");
  fSensor.replace(F("_DEV_CLA_"), "frequency");
  fSensor.replace(F("_STATE_TOPIC_"), config.Mqtt.MetricsTopic);
  fSensor.replace(F("_VAL_TEMPL_"), ("{{value_json.Freq}}"));
  fSensor.replace(F("_UNIQ_ID_"), devId + "_sensor_freq");
  fSensor.replace(F("_NAME_"), "Freq");
  fSensor.replace(F("_OBJ_ID_"), devId + "_sensor_freq");
  mqttClient.publish(("homeassistant/sensor/sensor_freq/" + devId + "/config").c_str(), fSensor.c_str());

  for (int i = 1; i <= 3; i++)
  {
    String iSensor = FPSTR(mqtt_sensor_template);
    iSensor.replace(F("_DEV_JSON_"), deviceInfo);
    iSensor.replace(F("_AVAILABILITY_"), availabilityBlock);
    iSensor.replace(F("_UNIT_"), "A");
    iSensor.replace(F("_DEV_CLA_"), "current");
    iSensor.replace(F("_STATE_TOPIC_"), config.Mqtt.MetricsTopic);
    iSensor.replace(F("_VAL_TEMPL_"), ("{{value_json.I" + String(i) + "}}"));
    iSensor.replace(F("_UNIQ_ID_"), devId + "_sensor_i" + String(i));
    iSensor.replace(F("_NAME_"), "I" + String(i));
    iSensor.replace(F("_OBJ_ID_"), devId + "_sensor_i" + String(i));
    mqttClient.publish(("homeassistant/sensor/sensor_i" + String(i) + "/" + devId + "/config").c_str(), iSensor.c_str());

    String vSensor = FPSTR(mqtt_sensor_template);
    vSensor.replace(F("_DEV_JSON_"), deviceInfo);
    vSensor.replace(F("_AVAILABILITY_"), availabilityBlock);
    vSensor.replace(F("_UNIT_"), "V");
    vSensor.replace(F("_DEV_CLA_"), "voltage");
    vSensor.replace(F("_STATE_TOPIC_"), config.Mqtt.MetricsTopic);
    vSensor.replace(F("_VAL_TEMPL_"), ("{{value_json.V" + String(i) + "}}"));
    vSensor.replace(F("_UNIQ_ID_"), devId + "_sensor_v" + String(i));
    vSensor.replace(F("_NAME_"), "V" + String(i));
    vSensor.replace(F("_OBJ_ID_"), devId + "_sensor_v" + String(i));
    mqttClient.publish(("homeassistant/sensor/sensor_v" + String(i) + "/" + devId + "/config").c_str(), vSensor.c_str());
  }
}