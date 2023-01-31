#include "html_init.h"
#include "html_common.h"
#include "html_main.h"
#include "html_pages.h"
#include "html_fieldsets.h"
#include "js.h"
#include "text.h"

class WebHandler
{

public:
    void Set(WebServer &server, MirtekGWConfig &config)
    {
        _server = &server;
        _config = &config;
    }

    void StartCaptiveMode(void (*onSave)(), void (*onReboot)())
    {
        _server->on("/", [this]()
                    { _InitCaptiveMode(); });
        _server->on("/save", HTTP_POST, [this]()
                    { _SaveCaptiveMode(); });
        _server->onNotFound([this]()
                            { _NotFound(); });
        _server->begin();
        _OnCaptiveSaveCallback = onSave;
        _OnCaptiveRebootCallback = onReboot;
    }

    void StartNormalMode()
    {
        _server->on("/", [this]()
                    { _HandleRoot(); });
        //// server.on("/control", handleControl);
        _server->on("/setup", [this]()
                    { _HandleSetup(); });
        _server->on("/mqtt", [this]()
                    { _HandleMqtt(); });
        _server->on("/wifi", [this]()
                    { _HandleWifi(); });
        _server->on("/meter", [this]()
                    { _HandleMeter(); });
        _server->on("/metrics", [this]()
                    { _HandleMetrics(); });
        // server.on("/unit", handleUnit);
        // server.on("/status", handleStatus);
        // server.on("/others", handleOthers);
        _server->on("/upgrade", [this]()
                    { _HandleUpgrade(); });
        _server->on(
            "/upload", HTTP_POST, [this]()
            { _HandleUploadDone(); },
            [this]()
            { _HandleUploadLoop(); });
        _server->on("/reboot", HTTP_POST, [this]()
                    { _HandleReboot(); });
        _server->on("/data5", HTTP_POST, [this]()
                    { _HandleData5(); });
        _server->on("/data9", HTTP_POST, [this]()
                    { _HandleData9(); });
        _server->onNotFound([this]()
                            { _NotFound(); });
        _server->begin();
    }

    void AttachMetricsCallback(String (*callback)())
    {
        _OnMetricsCallback = callback;
    }

    void AttachResetCallback(void (*callback)())
    {
        _OnResetCallback = callback;
    }

    void AttachRebootCallback(void (*callback)())
    {
        _OnRebootCallback = callback;
    }

    void AttachMeterSaveCallback(void (*callback)())
    {
        _OnMeterSaveCallback = callback;
    }

    void AttachMqttSaveCallback(void (*callback)())
    {
        _OnMqttSaveCallback = callback;
    }

    void AttachWifiSaveCallback(void (*callback)())
    {
        _OnWifiSaveCallback = callback;
    }

    void AttachUploadGetStatusCallback(int (*callback)())
    {
        _OnUploadGetStatusCallback = callback;
    }

    void AttachUploadRebootCallback(void (*callback)())
    {
        _OnUploadRebootCallback = callback;
    }

    void AttachUpgradeStartCallback(void (*callback)())
    {
        _OnUpgradeStartCallback = callback;
    }

    void AttachUploadLoopCallback(void (*callback)())
    {
        _OnUploadLoopCallback = callback;
    }

    void AttachData(Data5 &data5, Data9 &data9)
    {
        _data5 = &data5;
        _data9 = &data9;
    }

    void AttachRequestCallback(void (*callback5)(), void (*callback9)())
    {
        _OnRequest5 = callback5;
        _OnRequest9 = callback9;
    }

private:
    Data5 *_data5;
    Data9 *_data9;
    WebServer *_server;
    MirtekGWConfig *_config;
    std::function<void(void)> _OnCaptiveSaveCallback;
    std::function<void(void)> _OnCaptiveRebootCallback;
    std::function<String(void)> _OnMetricsCallback;
    std::function<void(void)> _OnResetCallback;
    std::function<void(void)> _OnRebootCallback;
    std::function<void(void)> _OnMeterSaveCallback;
    std::function<void(void)> _OnMqttSaveCallback;
    std::function<void(void)> _OnWifiSaveCallback;
    std::function<int(void)> _OnUploadGetStatusCallback;
    std::function<void(void)> _OnUploadRebootCallback;
    std::function<void(void)> _OnUploadLoopCallback;
    std::function<void(void)> _OnUpgradeStartCallback;
    std::function<void(void)> _OnRequest5;
    std::function<void(void)> _OnRequest9;

    void _NotFound()
    {
        _server->sendHeader("Location", "/");
        _server->sendHeader("Cache-Control", "no-cache");
        _server->send(302);
        return;
    }

    void _SaveCaptiveMode()
    {
        if (_server->arg("submit") == "reboot")
        {
            String initRebootPage = FPSTR(html_init_reboot);
            initRebootPage.replace("_TXT_INIT_REBOOT_", FPSTR(txt_init_reboot));
            _SendWrappedHTML(initRebootPage);

            if (_OnCaptiveRebootCallback)
            {
                _OnCaptiveRebootCallback();
            }
            else
            {
                Serial.println("Captive mode reboot callback not set");
            }
            return;
        }
        if (_server->arg("submit") == "submit")
        {
            _config->FillMeter(_server->arg("ma"));
            _config->FillWifi(_server->arg("ssid"), _server->arg("psk"), _server->arg("hn"), _server->arg("otapwd"));
            _config->FillMqtt(_server->arg("fn"), _server->arg("mh"), _server->arg("ml"), _server->arg("mu"), _server->arg("mp"), _server->arg("mt"));

            String initSavePage = FPSTR(html_init_save);
            initSavePage.replace("_TXT_INIT_REBOOT_MESS_", FPSTR(txt_init_reboot_mes));
            _SendWrappedHTML(initSavePage);

            if (_OnCaptiveSaveCallback)
            {
                _OnCaptiveSaveCallback();
            }
            else
            {
                Serial.println("Captive mode save callback not set");
            }
            return;
        }
        _NotFound();
    }

    void _InitCaptiveMode()
    {
        String initSetupPage = FPSTR(html_init_setup_header);
        initSetupPage += FPSTR(html_fieldset_meter);
        initSetupPage += FPSTR(html_fieldset_wifi);
        initSetupPage += FPSTR(html_fieldset_mqtt);
        initSetupPage += FPSTR(html_init_setup_footer);

        initSetupPage.replace("_TXT_METER_TITLE_", FPSTR(txt_meter_title));
        initSetupPage.replace("_TXT_METER_ADDRESS_", FPSTR(txt_meter_address));

        initSetupPage.replace("_TXT_WIFI_TITLE_", FPSTR(txt_wifi_title));
        initSetupPage.replace("_TXT_WIFI_HOST_", FPSTR(txt_wifi_hostname));
        initSetupPage.replace("_TXT_WIFI_SSID_", FPSTR(txt_wifi_SSID));
        initSetupPage.replace("_TXT_WIFI_PSK_", FPSTR(txt_wifi_psk));
        initSetupPage.replace("_TXT_WIFI_OTA_", FPSTR(txt_wifi_otap));

        initSetupPage.replace("_TXT_MQTT_TITLE_", FPSTR(txt_mqtt_title));
        initSetupPage.replace("_TXT_MQTT_FN_", FPSTR(txt_mqtt_fn));
        initSetupPage.replace("_TXT_MQTT_HOST_", FPSTR(txt_mqtt_host));
        initSetupPage.replace("_TXT_MQTT_PORT_", FPSTR(txt_mqtt_port));
        initSetupPage.replace("_TXT_MQTT_USER_", FPSTR(txt_mqtt_user));
        initSetupPage.replace("_TXT_MQTT_PASSWORD_", FPSTR(txt_mqtt_password));
        initSetupPage.replace("_TXT_MQTT_TOPIC_", FPSTR(txt_mqtt_topic));

        initSetupPage.replace(F("_METER_ADDRESS_"), (String)_config->Meter.MeterAddress);

        String str_ap_ssid = _config->Wifi.SSID;
        String str_ap_pwd = _config->Wifi.Password;
        String str_ota_pwd = _config->Wifi.OtaPassword;
        str_ap_ssid.replace("'", F("&apos;"));
        str_ap_pwd.replace("'", F("&apos;"));
        str_ota_pwd.replace("'", F("&apos;"));

        initSetupPage.replace(F("_WIFI_UNIT_NAME_"), _config->Wifi.Hostname);
        initSetupPage.replace(F("_WIFI_SSID_"), str_ap_ssid);
        initSetupPage.replace(F("_WIFI_PSK_"), str_ap_pwd);
        initSetupPage.replace(F("_WIFI_OTA_PWD_"), str_ota_pwd);

        initSetupPage.replace(F("_MQTT_FN_"), _config->Mqtt.FriendlyName);
        initSetupPage.replace(F("_MQTT_HOST_"), _config->Mqtt.Server);
        initSetupPage.replace(F("_MQTT_PORT_"), _config->Mqtt.Port);
        initSetupPage.replace(F("_MQTT_USER_"), _config->Mqtt.Username);
        initSetupPage.replace(F("_MQTT_PASSWORD_"), _config->Mqtt.Password);
        initSetupPage.replace(F("_MQTT_TOPIC_"), _config->Mqtt.Topic);

        initSetupPage.replace("_TXT_SAVE_", FPSTR(txt_save));
        initSetupPage.replace("_TXT_REBOOT_", FPSTR(txt_reboot));

        _SendWrappedHTML(initSetupPage);
    }

    void _HandleMetrics()
    {
        if (_OnMetricsCallback)
            _server->send(200, "text/plain", _OnMetricsCallback());
        else
            Serial.println("Metrics callback not set");
    }

    void _HandleRoot()
    {
        String menuRootPage = FPSTR(html_menu_root);
        menuRootPage.replace("_CURRENT_DATA_5_", _GetData5Text());
        menuRootPage.replace("_CURRENT_DATA_9_", _GetData9Text());
        menuRootPage.replace("_REQUEST_DATA_5_", FPSTR(txt_request_data_5));
        menuRootPage.replace("_REQUEST_DATA_9_", FPSTR(txt_request_data_9));
        menuRootPage.replace("_TXT_SETUP_", FPSTR(txt_setup));
        menuRootPage.replace("_TXT_STATUS_", FPSTR(txt_status));
        menuRootPage.replace("_TXT_FW_UPGRADE_", FPSTR(txt_firmware_upgrade));
        menuRootPage.replace("_TXT_REBOOT_", FPSTR(txt_reboot));
        _SendWrappedHTML(menuRootPage);
    }

    void _HandleSetup()
    {
        if (_server->hasArg("RESET") && _server->method() == HTTP_POST)
        {
            String pageReset = FPSTR(html_page_reset);
            String ssid = _config->DefaultHostname();
            pageReset.replace("_TXT_M_RESET_", FPSTR(txt_m_reset));
            pageReset.replace("_SSID_", ssid);
            _SendWrappedHTML(pageReset);

            if (_OnResetCallback)
            {
                _OnResetCallback();
            }
            else
            {
                Serial.println("Reset callback not set");
            }
        }
        else
        {
            String menuSetupPage = FPSTR(html_menu_setup);
            menuSetupPage.replace("_TXT_METER_", FPSTR(txt_METER));
            menuSetupPage.replace("_TXT_MQTT_", FPSTR(txt_MQTT));
            menuSetupPage.replace("_TXT_WIFI_", FPSTR(txt_WIFI));
            menuSetupPage.replace("_TXT_RESET_", FPSTR(txt_reset));
            menuSetupPage.replace("_TXT_BACK_", FPSTR(txt_back));
            menuSetupPage.replace("_TXT_RESETCONFIRM_", FPSTR(txt_reset_confirm));
            _SendWrappedHTML(menuSetupPage);
        }
    }

    void _HandleReboot()
    {
        _SendRebootPage();
        if (_OnRebootCallback)
        {
            _OnRebootCallback();
        }
        else
        {
            Serial.println("Reboot callback not set");
        }
    }

    void _HandleMeter()
    {
        if (_server->method() == HTTP_POST)
        {
            _config->FillMeter(_server->arg("ma"));
            _SendRebootPage();
            if (_OnMeterSaveCallback)
            {
                _OnMeterSaveCallback();
            }
            else
            {
                Serial.println("Meter save callback not set");
            }
        }
        else
        {
            String meterPage = FPSTR(html_page_common_header);
            meterPage += FPSTR(html_fieldset_meter);
            meterPage += FPSTR(html_page_common_footer);
            meterPage.replace("_TXT_SAVE_", FPSTR(txt_save));
            meterPage.replace("_TXT_BACK_", FPSTR(txt_back));
            meterPage.replace("_TXT_METER_TITLE_", FPSTR(txt_meter_title));
            meterPage.replace("_TXT_METER_ADDRESS_", FPSTR(txt_meter_address));
            meterPage.replace(F("_METER_ADDRESS_"), (String)_config->Meter.MeterAddress);
            _SendWrappedHTML(meterPage);
        }
    }

    void _HandleMqtt()
    {
        if (_server->method() == HTTP_POST)
        {

            _config->FillMqtt(_server->arg("fn"), _server->arg("mh"), _server->arg("ml"), _server->arg("mu"), _server->arg("mp"), _server->arg("mt"));

            _SendRebootPage();
            if (_OnMqttSaveCallback)
            {
                _OnMqttSaveCallback();
            }
            else
            {
                Serial.println("Mqtt save callback not set");
            }
        }
        else
        {

            String mqttPage = FPSTR(html_page_common_header);
            mqttPage += FPSTR(html_fieldset_mqtt);
            mqttPage += FPSTR(html_page_common_footer);

            // String mqttPage = FPSTR(html_page_mqtt);
            mqttPage.replace("_TXT_SAVE_", FPSTR(txt_save));
            mqttPage.replace("_TXT_BACK_", FPSTR(txt_back));
            mqttPage.replace("_TXT_MQTT_TITLE_", FPSTR(txt_mqtt_title));
            mqttPage.replace("_TXT_MQTT_FN_", FPSTR(txt_mqtt_fn));
            mqttPage.replace("_TXT_MQTT_HOST_", FPSTR(txt_mqtt_host));
            mqttPage.replace("_TXT_MQTT_PORT_", FPSTR(txt_mqtt_port));
            mqttPage.replace("_TXT_MQTT_USER_", FPSTR(txt_mqtt_user));
            mqttPage.replace("_TXT_MQTT_PASSWORD_", FPSTR(txt_mqtt_password));
            mqttPage.replace("_TXT_MQTT_TOPIC_", FPSTR(txt_mqtt_topic));
            mqttPage.replace(F("_MQTT_FN_"), _config->Mqtt.FriendlyName);
            mqttPage.replace(F("_MQTT_HOST_"), _config->Mqtt.Server);
            mqttPage.replace(F("_MQTT_PORT_"), _config->Mqtt.Port);
            mqttPage.replace(F("_MQTT_USER_"), _config->Mqtt.Username);
            mqttPage.replace(F("_MQTT_PASSWORD_"), _config->Mqtt.Password);
            mqttPage.replace(F("_MQTT_TOPIC_"), _config->Mqtt.Topic);
            _SendWrappedHTML(mqttPage);
        }
    }

    void _HandleWifi()
    {
        if (_server->method() == HTTP_POST)
        {
            _config->FillWifi(_server->arg("ssid"), _server->arg("psk"), _server->arg("hn"), _server->arg("otapwd"));
            _SendRebootPage();
            if (_OnWifiSaveCallback)
            {
                _OnWifiSaveCallback();
            }
            else
            {
                Serial.println("Wifi save callback not set");
            }
        }
        else
        {
            String wifiPage = FPSTR(html_page_common_header);
            wifiPage += FPSTR(html_fieldset_wifi);
            wifiPage += FPSTR(html_page_common_footer);
            String str_ap_ssid = _config->Wifi.SSID;
            String str_ap_pwd = _config->Wifi.Password;
            String str_ota_pwd = _config->Wifi.OtaPassword;
            str_ap_ssid.replace("'", F("&apos;"));
            str_ap_pwd.replace("'", F("&apos;"));
            str_ota_pwd.replace("'", F("&apos;"));
            wifiPage.replace("_TXT_SAVE_", FPSTR(txt_save));
            wifiPage.replace("_TXT_BACK_", FPSTR(txt_back));
            wifiPage.replace("_TXT_WIFI_TITLE_", FPSTR(txt_wifi_title));
            wifiPage.replace("_TXT_WIFI_HOST_", FPSTR(txt_wifi_hostname));
            wifiPage.replace("_TXT_WIFI_SSID_", FPSTR(txt_wifi_SSID));
            wifiPage.replace("_TXT_WIFI_PSK_", FPSTR(txt_wifi_psk));
            wifiPage.replace("_TXT_WIFI_OTA_", FPSTR(txt_wifi_otap));
            wifiPage.replace(F("_WIFI_UNIT_NAME_"), _config->Wifi.Hostname);
            wifiPage.replace(F("_WIFI_SSID_"), str_ap_ssid);
            wifiPage.replace(F("_WIFI_PSK_"), str_ap_pwd);
            wifiPage.replace(F("_WIFI_OTA_PWD_"), str_ota_pwd);
            _SendWrappedHTML(wifiPage);
        }
    }

    void _HandleUpgrade()
    {
        if (_OnUpgradeStartCallback)
            _OnUpgradeStartCallback();
        else
            Serial.println("Upgrade start callback not set");
        String upgradePage = FPSTR(html_page_upgrade);
        upgradePage.replace("_TXT_B_UPGRADE_", FPSTR(txt_upgrade));
        upgradePage.replace("_TXT_BACK_", FPSTR(txt_back));
        upgradePage.replace("_TXT_UPGRADE_TITLE_", FPSTR(txt_upgrade_title));
        upgradePage.replace("_TXT_UPGRADE_INFO_", FPSTR(txt_upgrade_info));
        upgradePage.replace("_TXT_UPGRADE_START_", FPSTR(txt_upgrade_start));

        _SendWrappedHTML(upgradePage);
    }

    void _HandleUploadDone()
    {
        int uploadError = _OnUploadGetStatusCallback();
        bool reset = false;
        String uploadPage = FPSTR(html_page_upload);
        String content = F("<div style='text-align:center;'><b>Upload</b> ");
        if (uploadError == 0)
        {
            content += F("<span style='color:#47c266; font-weight: bold;'>");
            content += FPSTR(txt_upload_sucess);
            content += F("</span><br/><br/>");
            content += FPSTR(txt_upload_refresh);
            content += F(" <span id='count'>10s</span>...");
            content += FPSTR(count_down_script);
            reset = true;
        }
        else
        {
            content += F("<span style='color:#d43535'>failed</span></b><br/><br/>");
            if (uploadError == 1)
            {
                content += FPSTR(txt_upload_nofile);
            }
            else if (uploadError == 2)
            {
                content += FPSTR(txt_upload_filetoolarge);
            }
            else if (uploadError == 3)
            {
                content += FPSTR(txt_upload_fileheader);
            }
            else if (uploadError == 4)
            {
                content += FPSTR(txt_upload_flashsize);
            }
            else if (uploadError == 5)
            {
                content += FPSTR(txt_upload_buffer);
            }
            else if (uploadError == 6)
            {
                content += FPSTR(txt_upload_failed);
            }
            else if (uploadError == 7)
            {
                content += FPSTR(txt_upload_aborted);
            }
            else
            {
                content += FPSTR(txt_upload_error);
                content += String(uploadError);
            }
            if (Update.hasError())
            {
                content += " ";
                content += FPSTR(txt_upload_code);
                content += String(Update.getError());
            }
        }
        content += F("</div><br/>");
        uploadPage.replace("_UPLOAD_MSG_", content);
        uploadPage.replace("_TXT_BACK_", FPSTR(txt_back));
        _SendWrappedHTML(uploadPage);
        if (reset)
        {
            if (_OnUploadRebootCallback)
            {
                _OnUploadRebootCallback();
            }
            else
            {
                Serial.println("Upload reboot callback not set");
            }
        }
    }

    void _HandleUploadLoop()
    {
        _OnUploadLoopCallback();
    }

    void _SendWrappedHTML(String &content)
    {
        String headerContent = FPSTR(html_common_header);
        String footerContent = FPSTR(html_common_footer);
        String toSend = headerContent + content + footerContent;
        toSend.replace(F("_UNIT_NAME_"), _config->Wifi.Hostname);
        if (_config->Meter.MeterAddress > 0)
        {
            toSend.replace(F("_METER_ADDRESS_"), String(_config->Meter.MeterAddress));
        }
        else
        {
            toSend.replace(F("_METER_ADDRESS_"), "");
        }
        toSend.replace(F("_VERSION_"), _mirtekgw2_version);
        _server->send(200, F("text/html"), toSend);
    }

    void _SendRebootPage()
    {
        String saveRebootPage = FPSTR(html_page_save_reboot);
        String countDown = FPSTR(count_down_script);
        saveRebootPage.replace("_TXT_M_SAVE_", FPSTR(txt_m_save));
        _SendWrappedHTML(saveRebootPage + countDown);
    }

    String _GetData5Text()
    {
        if (_data5->Updated > 0)
        {
            String res = FPSTR(html_data5);
            res.replace("_UPDATED_", _GetMillisToText(millis() - _data5->Updated));
            res.replace("_SUM_", String(_data5->Sum));
            res.replace("_T1_", String(_data5->T1));
            res.replace("_T2_", String(_data5->T2));
            return res;
        }
        else
            return "";
    }

    String _GetData9Text()
    {
        if (_data9->Updated > 0)
        {
            String res = FPSTR(html_data9);
            res.replace("_UPDATED_", _GetMillisToText(millis() - _data9->Updated));
            res.replace("_I1_", String(_data9->I1));
            res.replace("_I2_", String(_data9->I2));
            res.replace("_I3_", String(_data9->I3));
            res.replace("_V1_", String(_data9->V1));
            res.replace("_V2_", String(_data9->V2));
            res.replace("_V3_", String(_data9->V3));
            res.replace("_COS_", String(_data9->Cos));
            res.replace("_FREQ_", String(_data9->Freq));
            return res;
        }
        else
            return "";
    }

    String _GetMillisToText(unsigned long m)
    {
        String s = "";
        unsigned long days = floor(m / 1000.0 / 60.0 / 60.0 / 24.0);
        unsigned long hours = floor(m % (1000 * 60 * 60 * 24) / 1000.0 / 60.0 / 60.0);
        unsigned long min = floor(m % (1000 * 60 * 60) / 1000.0 / 60.0);
        unsigned long sec = floor(m % (1000 * 60) / 1000.0);
        if (days > 0)
            s += String(days) + "d ";
        if (hours > 0)
            s += String(hours) + "h ";
        if (min > 0)
            s += String(min) + "m ";
        if (sec > 0)
            s += String(sec) + "s";
        return s;
    }

    void _HandleData5()
    {
        if (_OnRequest5)
            _OnRequest5();
        String saveRebootPage = FPSTR(html_page_request);
        String countDown = FPSTR(count_down_script);
        saveRebootPage.replace("_TXT_M_REQUEST_", FPSTR(txt_m_request));
        _SendWrappedHTML(saveRebootPage + countDown);
    }

    void _HandleData9()
    {
        if (_OnRequest9)
            _OnRequest9();
        String saveRebootPage = FPSTR(html_page_request);
        String countDown = FPSTR(count_down_script);
        saveRebootPage.replace("_TXT_M_REQUEST_", FPSTR(txt_m_request));
        _SendWrappedHTML(saveRebootPage + countDown);
    }
};