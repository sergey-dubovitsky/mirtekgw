#include <pgmspace.h>

const char mqtt_device_template[] PROGMEM = R"====("dev":{"cu":"_DEV_URL_","ids":["_DEV_ID_"],"name":"_DEV_NAME_","sw_version":"_DEV_SW_VER_","mdl":"Mirtek GW","mf":""})====";

const char mqtt_availability_payload[] PROGMEM = R"====("avty_t":"_AV_TOPIC_","pl_avail":"_AV_ON_","pl_not_avail":"_AV_OFF_")====";

const char mqtt_button_config_template[] PROGMEM = R"====({"pl_prs":"_PAYLOAD_","cmd_t":"_CMD_TOPIC_","uniq_id":"_UNIQ_ID_","name":"_TEXT_","obj_id":"_OBJ_ID_","ent_cat":"config",_DEV_JSON_,_AVAILABILITY_})====";

const char mqtt_button_template[] PROGMEM = R"====({"pl_prs":"_PAYLOAD_","cmd_t":"_CMD_TOPIC_","uniq_id":"_UNIQ_ID_","name":"_TEXT_","obj_id":"_OBJ_ID_",_DEV_JSON_,_AVAILABILITY_})====";

const char mqtt_sensor_template[] PROGMEM = R"====({"unit_of_meas":"_UNIT_","dev_cla":"_DEV_CLA_","stat_t":"_STATE_TOPIC_","val_tpl":"_VAL_TEMPL_","uniq_id":"_UNIQ_ID_","name":"_NAME_","obj_id":"_OBJ_ID_","stat_cla": "measurement",_DEV_JSON_,_AVAILABILITY_})====";

//const char mqtt_sensor_no_unit_template[] PROGMEM = R"====({"dev_cla":"_DEV_CLA_","stat_t":"_STATE_TOPIC_","val_tpl":"_VAL_TEMPL_","uniq_id":"_UNIQ_ID_","name":"_NAME_","obj_id":"_OBJ_ID_","stat_cla":"measurement",_DEV_JSON_,_AVAILABILITY_})====";
