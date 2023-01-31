#include <pgmspace.h>

const char html_page_common_header[] PROGMEM = R"====(
<form method='post'>
)====";

const char html_page_common_footer[] PROGMEM =R"====(
  <div>
    <button name='save' type='submit' class='button bgrn'>_TXT_SAVE_</button>
    <p></p>
    <a class='button back' href='/setup'>_TXT_BACK_</a>
   </div>
</form>
)====";

const char html_page_save_reboot[] PROGMEM =
"<p>_TXT_M_SAVE_ <span id='count'>10s</span>...</p>"
;

const char html_page_reset[] PROGMEM =
"<p>_TXT_M_RESET_ _SSID_...</p>"
;

const char html_page_upgrade[] PROGMEM =R"====(
<script>function eb(s) {return document.getElementById(s);}</script>
<div id='f1' style='display:block;'>
<form method='post' action='upload' enctype='multipart/form-data'>
<fieldset><legend><b>&nbsp; _TXT_UPGRADE_TITLE_ &nbsp;</b></legend><p><span>_TXT_UPGRADE_INFO_</span></p><br><input type='file' accept='.bin' name='upload'><br><br></fieldset>
<div><button type='submit' onclick="eb('f1').style.display='none';eb('f2').style.display='block';this.form.submit();" class='button bgrn'>_TXT_B_UPGRADE_</button><p></p><a class='button back' href='/'>_TXT_BACK_</a></div>
</form>
</div>
<div id='f2' style='display:none;text-align:center;'><b>_TXT_UPGRADE_START_ ...</b></div>
)====";

const char html_page_upload[] PROGMEM =R"====(
<div id='l1' name='l1'><br>_UPLOAD_MSG_<p><a class='button back' href='/'>_TXT_BACK_</a></p></div>
)====";

const char html_page_request[] PROGMEM =
"<p>_TXT_M_REQUEST_ <span id='count'>10s</span>...</p>"
;