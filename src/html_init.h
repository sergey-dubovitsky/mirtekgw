const char html_init_setup_header[] PROGMEM = R"====(
<form method='post' action='save'>
)====";

const char html_init_setup_footer[] PROGMEM = R"====(
  <div>
    <button name='submit' value='submit' type='submit' class='button bgrn'> _TXT_SAVE_ </button>
    <p></p>
    <button name='submit' value='reboot' type='submit' class='back'> _TXT_REBOOT_ </button>
   </div>
</form>
)====";

const char html_init_reboot[] PROGMEM =  R"====(
<p> _TXT_INIT_REBOOT_ </p>
)====";

const char html_init_save[] PROGMEM =  R"====(
<p> _TXT_INIT_REBOOT_MESS_ </p>
)====";