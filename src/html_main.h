const char html_menu_root[] PROGMEM = R"====(
_CURRENT_DATA_5_
_CURRENT_DATA_9_
<div id="control">
    <form action='/data5' method='post'>
        <button>_REQUEST_DATA_5_</button>
    </form>
</div>
<div id="control">
    <form action='/data9' method='post'>
        <button>_REQUEST_DATA_9_</button>
    </form>
</div>
<div>
    <a class='button' href='/setup'>_TXT_SETUP_</a>
</div>
<div>
    <form action='/upgrade' method='get'>
        <button>_TXT_FW_UPGRADE_</button>
    </form>
</div>
<div>
    <form action='/reboot' method='post'>
        <button type='submit' name='REBOOT' class='button bred'>_TXT_REBOOT_</button>
    </form>
</div>
)====";


const char html_menu_setup[] PROGMEM = R"====(
<p>
    <a class='button' href='/meter'>_TXT_METER_</a>
</p>
<p>
    <a class='button' href='/wifi'>_TXT_WIFI_</a>
</p>
<p>
    <a class='button' href='/mqtt'>_TXT_MQTT_</a>
</p>
<p>
    <form onsubmit="return confirm(' _TXT_RESETCONFIRM_ ');" method='POST'>
        <button type='submit' name='RESET' class='button bred'>_TXT_RESET_</button>
    </form>
</p>
<p>
    <form action='/' method='get'>
        <button>_TXT_BACK_</button>
    </form>
</p>
)====";
