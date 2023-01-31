#include <pgmspace.h>

const char html_fieldset_meter[] PROGMEM = R"====(
  <div>
    <fieldset>
      <legend><b>&nbsp; _TXT_METER_TITLE_ &nbsp;</b></legend>
      <p><b>_TXT_METER_ADDRESS_</b> (*)
        <br/>
        <input type='number' id='ma' name='ma' min='0' max='65535' value='_METER_ADDRESS_'>
      </p>
    </fieldset>
  </div>
)====";

const char html_fieldset_wifi[] PROGMEM = R"====(
  <div>
    <fieldset>
      <legend><b>&nbsp; _TXT_WIFI_TITLE_ &nbsp;</b></legend>  
        <p><b>_TXT_WIFI_HOST_</b>
          <br/>
          <input id='hn' name='hn' placeholder='' value='_WIFI_UNIT_NAME_'>
        </p>  
        <p><b>_TXT_WIFI_SSID_</b> (*)
          <br/>
          <input id='ssid' name='ssid' placeholder='' autocomplete='off' autocorrect='off' autocapitalize='off' spellcheck='false' value='_WIFI_SSID_'>
        </p>
        <p><b>_TXT_WIFI_PSK_</b> (*)
          <br/>
          <input id='psk' name='psk' placeholder='' autocomplete='off' autocorrect='off' autocapitalize='off' spellcheck='false' value='_WIFI_PSK_'>
        </p>
        <!--<p><b>_TXT_WIFI_OTA_</b>
          <br/>
          <input id='otapwd' name='otapwd' placeholder='' autocomplete='off' autocorrect='off' autocapitalize='off' spellcheck='false' value='_WIFI_OTA_PWD_'>
        </p>-->
    </fieldset>
  </div>
)====";

const char html_fieldset_mqtt[] PROGMEM = R"====(
  <div>
    <fieldset>
      <legend><b>&nbsp; _TXT_MQTT_TITLE_ &nbsp;</b></legend>
        <p><b>_TXT_MQTT_FN_</b>
            <br/>
            <input id='fn' name='fn' autocomplete='off' autocorrect='off' autocapitalize='off' spellcheck='false' placeholder='' value='_MQTT_FN_'>
        </p>
        <p><b>_TXT_MQTT_HOST_</b>
            <br/>
            <input id='mh' name='mh' autocomplete='off' autocorrect='off' autocapitalize='off' spellcheck='false' placeholder='' value='_MQTT_HOST_'>
        </p>
        <p><b>_TXT_MQTT_PORT_</b>
            <br/>
            <input id='ml' name='ml' type='number' placeholder='1883' autocomplete='off' autocorrect='off' autocapitalize='off' spellcheck='false' min='0' max='65535' value='_MQTT_PORT_'>
        </p>
        <p><b>_TXT_MQTT_USER_</b>
            <br/>
            <input id='mu' name='mu' autocomplete='off' autocorrect='off' autocapitalize='off' spellcheck='false' placeholder='' value='_MQTT_USER_'>
        </p>
        <p><b>_TXT_MQTT_PASSWORD_</b>
            <br/>
            <input id='mp' name='mp' placeholder='' autocomplete='off' autocorrect='off' autocapitalize='off' spellcheck='false' value='_MQTT_PASSWORD_'>
        </p>
        <p><b>_TXT_MQTT_TOPIC_</b>
            <br/>
            <input id='mt' name='mt' autocomplete='off' autocorrect='off' autocapitalize='off' spellcheck='false' placeholder='topic' value='_MQTT_TOPIC_'>
        </p>
    </fieldset>
  </div>
)====";

const char html_data5[] PROGMEM = R"====(
<style>
.d5v { width: 100px; text-align: right; }
</style>
<table style='width: 100%'>
  <tr>
    <td><b>Показания, кВт*ч</b></td>
    <td class='d5v'>🕗 _UPDATED_</td>
  </tr>
  <tr>
    <td>Sum</td>
    <td class='d5v'>_SUM_</td>
  </tr>
  <tr>
    <td>T1</td>
    <td class='d5v'>_T1_</td>
  </tr>
  <tr>
    <td>T2</td>
    <td class='d5v'>_T2_</td>
  </tr>
</table>
<p/>
)====";

const char html_data9[] PROGMEM = R"====(
<style>
.d9v { width: 100px; text-align: right; }
</style>
<table style='width: 100%'>
  <tr>
    <td colspan='3'><b>Метрики</b></td>
    <td class='d9v'>🕗 _UPDATED_</td>
  </tr>
  <tr>
    <td>I, А</td>
    <td class='d9v'>_I1_</td>
    <td class='d9v'>_I2_</td>
    <td class='d9v'>_I3_</td>
  </tr>
  <tr>
    <td>V, В</td>
    <td class='d9v'>_V1_</td>
    <td class='d9v'>_V2_</td>
    <td class='d9v'>_V3_</td>
  </tr>
  <tr>
    <td colspan='3'>Freq, Гц</td>
    <td class='d9v'>_FREQ_</td>
  </tr>
  <tr>
    <td colspan='3'>Cos</td>
    <td class='d9v'>_COS_</td>
  </tr>
</table>
<p/>
)====";