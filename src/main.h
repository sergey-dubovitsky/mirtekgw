#include "WString.h"

typedef bool boolean;

boolean initWifi();

boolean initMqtt();

bool connectWifi();

void loopWiFiConnectivity();

void loopMqttConnect();

void handleMetrics();

void mqttSubscribe(char *topic, byte *payload, unsigned int length);

void tmrUptimeCallback();

bool initRadio();

void Log(String s);

bool ProcessPacket5();

bool ProcessPacket9();

void OnCaptiveSaveCallback();

void OnCaptiveRebootCallback();

void OnResetCallback();

void OnRebootCallback();

void OnMeterSaveCallback();

void OnMqttSaveCallback();

void OnWifiSaveCallback();

int OnUploadGetStatusCallback();

void OnUploadRebootCallback();

void OnUploadLoopCallback();

void OnUpgradeStartCallback();

void OnMeterData5Callback();

void OnMeterData9Callback();

void OnRequestData5Callback();

void OnRequestData9Callback();

void initHADiscovery();

DynamicJsonDocument GetDiscoveryJson(
    String &dicoveryTopic, String stateTopic, String availabilityTopic,
    String name, String fieldName, String jsonFieldName,
    String unitOfMeasurement, String deviceClass);