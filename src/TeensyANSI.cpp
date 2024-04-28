#include <SdFat.h>
#include <minIni.h>
#include <minIni_cache.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include "TeensyANSI_config.h"
#include "TeensyANSI_platform.h"
#include "TeensyANSI_log.h"
// #include "TeensyANSI_log_trace.h"
// #include "TeensyANSI_settings.h"
// #include "TeensyANSI_disk.h"
// #include "TeensyANSI_initiator.h"
// #include "TeensyANSI_msc.h"
// #include "ROMDrive.h"

// SdFs SD;
// FsFile g_logfile;
// static bool g_romdrive_active;
// static bool g_sdcard_present;

/************************************/
/* Status reporting by blinking led */
/************************************/

#define BLINK_STATUS_OK 1
#define BLINK_ERROR_NO_IMAGES  3
#define BLINK_DIRECT_MODE      4
#define BLINK_ERROR_NO_SD_CARD 5

void blinkStatus(int count)
{
  uint8_t blink_delay = 250;
  if (count == BLINK_DIRECT_MODE)
    blink_delay = 100;

  for (int i = 0; i < count; i++)
  {
    // LED_ON();
    delay(blink_delay);
    // LED_OFF();
    delay(blink_delay);
  }
}

// Place all the setup code that requires the SD card to be initialized here
// Which is pretty much everything after platform_init and and platform_late_init
static void teensyansi_setup_sd_card()
{    
}

extern "C" void teensyansi_setup(void)
{
  platform_init();
  platform_late_init();
  teensyansi_setup_sd_card();
}

extern "C" void teensyansi_main_loop(void)
{
}

