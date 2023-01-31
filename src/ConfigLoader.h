struct MeterConfig
{
  uint16_t MeterAddress = 0;
  bool Loaded = false;
  const PROGMEM char *FilePath = "/meter.json";
};

struct WifiConfig
{
  String Hostname = "";
  String SSID = "";
  String Password = "";
  String OtaPassword = "";
  IPAddress IpAddress = IPAddress(192, 168, 4, 1);
  IPAddress NetworkMask = IPAddress(255, 255, 255, 0);
  bool Loaded = false;
  const PROGMEM char *FilePath = "/wifi.json";
};

struct MQTTConfig
{
  String FriendlyName = "";
  String Server = "";
  String Port = "1883";
  String Username = "";
  String Password = "";
  String Topic = "";
  String AvailabilityTopic = "";
  String CommandRebootTopic = "";
  String UptimeTopic = "";
  String CommandRequestDataTopic = "";
  String ConsumptionTopic = "";
  String MetricsTopic = "";
  String DiscoveryTopic = "";
  bool Loaded = false;
  const PROGMEM char *FilePath = "/mqtt.json";
};

class MirtekGWConfig
{

public:
  MeterConfig Meter;
  WifiConfig Wifi;
  MQTTConfig Mqtt;

  MirtekGWConfig()
  {
    Serial.println("CL default constructor");
    Mqtt.FriendlyName = Mqtt.Topic = Wifi.Hostname = DefaultHostname();
  }
  
  void Load()
  {
    LoadMeterConfig();
    LoadWifiConfig();
    LoadMqttConfig();
  }

  void FillMqtt(String friendlyName, String host, String port, String user, String password, String topic)
  {
    if (port[0] == '\0')
      port = "1883";

    Mqtt.FriendlyName = friendlyName;
    Mqtt.Server = host;
    Mqtt.Port = port;
    Mqtt.Username = user;
    Mqtt.Password = password;
    Mqtt.Topic = topic;
  }

  void FillMeter(String meterAddress)
  {
    Meter.MeterAddress = (uint16_t)atoi(meterAddress.c_str());
  }

  void FillWifi(String apSsid, String apPwd, String hostName, String otaPwd)
  {
    Wifi.Hostname = hostName;
    Wifi.SSID = apSsid;
    Wifi.Password = apPwd;
    Wifi.OtaPassword = otaPwd;
  }

  void Save()
  {
    SaveMeter();
    SaveWifi();
    SaveMqtt();
  }

  void SaveMqtt()
  {
    const size_t capacity = JSON_OBJECT_SIZE(6) + 400;
    DynamicJsonDocument doc(capacity);
    doc["mqtt_fn"] = Mqtt.FriendlyName;
    doc["mqtt_host"] = Mqtt.Server;
    doc["mqtt_port"] = Mqtt.Port;
    doc["mqtt_user"] = Mqtt.Username;
    doc["mqtt_pwd"] = Mqtt.Password;
    doc["mqtt_topic"] = Mqtt.Topic;
    File configFile = SPIFFS.open(Mqtt.FilePath, "w");
    if (!configFile)
    {
      Serial.println(F("Failed to open config file for writing"));
    }
    serializeJson(doc, configFile);
    delay(10);
    configFile.close();
    doc.clear();
  }

  void SaveMeter()
  {
    const size_t capacity = JSON_OBJECT_SIZE(1) + 30;
    DynamicJsonDocument doc(capacity);
    doc["ma"] = Meter.MeterAddress;
    File configFile = SPIFFS.open(Meter.FilePath, "w");
    if (!configFile)
    {
      Serial.println(F("Failed to open meter file for writing"));
    }
    serializeJson(doc, configFile);
    delay(10);
    configFile.close();
    doc.clear();
  }

  void SaveWifi()
  {
    const size_t capacity = JSON_OBJECT_SIZE(4) + 130;
    DynamicJsonDocument doc(capacity);
    doc["ap_ssid"] = Wifi.SSID;
    doc["ap_pwd"] = Wifi.Password;
    doc["hostname"] = Wifi.Hostname;
    doc["ota_pwd"] = Wifi.OtaPassword;
    File configFile = SPIFFS.open(Wifi.FilePath, "w");
    if (!configFile)
    {
      Serial.println(F("Failed to open wifi file for writing"));
    }
    serializeJson(doc, configFile);
    delay(10);
    configFile.close();
    doc.clear();
  }

  String DefaultHostname()
  {
    uint64_t macAddress = ESP.getEfuseMac();
    uint32_t chipID = macAddress >> 24;
    return "mirtekgw_" + String(chipID, HEX);
  }

private:
  void LoadMeterConfig()
  {
    File configFile;
    if (!OpenConfigFile(configFile, Meter.FilePath))
    {
      return;
    }
    DynamicJsonDocument doc = ReadFileContent(configFile, 200);
    Meter.MeterAddress = doc["ma"].as<uint16_t>();
    Meter.Loaded = true;
    return;
  }

  void LoadWifiConfig()
  {
    File configFile;
    if (!OpenConfigFile(configFile, Wifi.FilePath))
    {
      return;
    }
    DynamicJsonDocument doc = ReadFileContent(configFile, 200);
    Wifi.Hostname = doc["hostname"].as<String>();
    Wifi.SSID = doc["ap_ssid"].as<String>();
    Wifi.Password = doc["ap_pwd"].as<String>();
    if (doc.containsKey("ota_pwd"))
    {
      Wifi.OtaPassword = doc["ota_pwd"].as<String>();
    }
    else
    {
      Wifi.OtaPassword = "";
    }
    Wifi.Loaded = true;
    return;
  }

  void LoadMqttConfig()
  {
    File configFile;
    if (!OpenConfigFile(configFile, Mqtt.FilePath))
    {
      return;
    }
    DynamicJsonDocument doc = ReadFileContent(configFile, 400);
    Mqtt.FriendlyName = doc["mqtt_fn"].as<String>();
    Mqtt.Server = doc["mqtt_host"].as<String>();
    Mqtt.Port = doc["mqtt_port"].as<String>();
    Mqtt.Username = doc["mqtt_user"].as<String>();
    Mqtt.Password = doc["mqtt_pwd"].as<String>();
    Mqtt.Topic = doc["mqtt_topic"].as<String>();
    Mqtt.AvailabilityTopic = Mqtt.Topic + "/availability";
    Mqtt.CommandRebootTopic = Mqtt.Topic + "/command/reboot";
    Mqtt.CommandRequestDataTopic = Mqtt.Topic + "/command/requestdata";
    Mqtt.UptimeTopic = Mqtt.Topic + "/uptime";
    Mqtt.ConsumptionTopic = Mqtt.Topic + "/consumption";
    Mqtt.MetricsTopic = Mqtt.Topic + "/metrics";
    //Mqtt.DiscoveryTopic = "homeassistant/sensor/" + Mqtt.FriendlyName + "/config";
    Mqtt.Loaded = true;
    return;
  }

  static bool OpenConfigFile(File &file, String fileName)
  {
    if (!SPIFFS.exists(fileName))
    {
      Serial.print(fileName);
      Serial.println(F(" config file not exist!"));
      return false;
    }
    file = SPIFFS.open(fileName, "r");
    if (!file)
    {
      Serial.print(fileName);
      Serial.println(F(" failed to open %s config file"));
      return false;
    }
    size_t size = file.size();
    if (size > 1024)
    {
      Serial.print(fileName);
      Serial.println(F(" config file size is too large"));
      return false;
    }
    Serial.print(fileName);
    Serial.println(F(" config file opened successfully"));
    return true;
  }

  static DynamicJsonDocument ReadFileContent(File &file, int cap)
  {
    size_t size = file.size();
    std::unique_ptr<char[]> buf(new char[size]);
    file.readBytes(buf.get(), size);
    const size_t capacity = JSON_OBJECT_SIZE(4) + cap;
    DynamicJsonDocument doc(capacity);
    deserializeJson(doc, buf.get());
    return doc;
  }
};
