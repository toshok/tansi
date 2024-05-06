#include <SdFat.h>
#include <SD.h>
#include <minIni.h>
#include <minIni_cache.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include "TANSI_config.h"
#include "TANSI_platform.h"
#include "TANSI_log.h"
#include "ansi.h"
// #include "TANSI_log_trace.h"
#include "TANSI_settings.h"
#include "TANSI_disk.h"
// #include "TANSI_msc.h"

extern minIni inifile;

FsFile g_logfile;
static bool g_sdcard_present;

void save_logfile(bool always = false)
{
  static uint32_t prev_log_pos = 0;
  static uint32_t prev_log_len = 0;
  static uint32_t prev_log_save = 0;
  uint32_t loglen = log_get_buffer_len();

  if (loglen != prev_log_len && g_sdcard_present)
  {
    // When debug is off, save log at most every LOG_SAVE_INTERVAL_MS
    // When debug is on, save after every ANSI command.
    if (always || g_log_debug || (LOG_SAVE_INTERVAL_MS > 0 && (uint32_t)(millis() - prev_log_save) > LOG_SAVE_INTERVAL_MS))
    {
      g_logfile.write(log_get_buffer(&prev_log_pos));
      g_logfile.flush();
      
      prev_log_len = loglen;
      prev_log_save = millis();
    }
  }
}

void init_logfile()
{
  static bool first_open_after_boot = true;

  bool truncate = first_open_after_boot;
  int flags = O_WRONLY | O_CREAT | (truncate ? O_TRUNC : O_APPEND);
  g_logfile = SD.sdfs.open(LOGFILE, flags);
  if (!g_logfile.isOpen())
  {
    logmsg("Failed to open log file: ", SD.sdfs.sdErrorCode());
  }
  save_logfile(true);

  first_open_after_boot = false;
}

void print_sd_info()
{
  uint64_t size = (uint64_t)SD.sdfs.vol()->clusterCount() * SD.sdfs.vol()->bytesPerCluster();
  logmsg("SD card detected, FAT", (int)SD.sdfs.vol()->fatType(),
          " volume size: ", (int)(size / 1024 / 1024), " MB");
  
  cid_t sd_cid;

  if (SD.sdfs.card()->readCID(&sd_cid)) {
    logmsg("SD MID: ", (uint8_t)sd_cid.mid, ", OID: ", (uint8_t)sd_cid.oid[0], " ", (uint8_t)sd_cid.oid[1]);
    
    char sdname[6] = {sd_cid.pnm[0], sd_cid.pnm[1], sd_cid.pnm[2], sd_cid.pnm[3], sd_cid.pnm[4], 0};
    logmsg("SD Name: ", sdname);
    logmsg("SD Date: ", (int)sd_cid.mdt_month, "/", sd_cid.mdt_year_low);
    logmsg("SD Serial: ", sd_cid.psn);
  }
}

/************************************/
/* Status reporting by blinking led */
/************************************/

#define BLINK_STATUS_OK        1
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

// Iterate over the root path in the SD card looking for candidate image files.
bool findHDDImages() {
  uint8_t idsSeen = 0; // bit mask of ANSI IDs seen

  std::string imgdir = inifile.gets("ANSI", "Dir", "/");

  logmsg("Finding images in directory ", imgdir, ":");

  SdFile root;
  root.open(imgdir.c_str());
  if (!root.isOpen()) {
    logmsg("Could not open directory: ", imgdir);
  }

  SdFile file;
  bool foundImage = false;
  while (1) {
    if (!file.openNext(&root, O_READ)) {
      break;
    }

    // Skip directories
    if(file.isDir()) {
      continue;
    }

    char name[MAX_FILE_PATH+1];
    file.getName(name, MAX_FILE_PATH+1);
    file.close();

    // Check if the file is a valid image file
    bool is_hd = strlen(name) >= 2 && (tolower(name[0]) == 'h' && tolower(name[1]) == 'd');
    if (!is_hd) {
      continue;
    }

    // skip file if the name indicates it is not a valid image container
    if (!ansiDiskFilenameValid(name)) {
      continue;
    }

    // Defaults for Hard Disks
    int id  = 0;

    // Parse ANSI device ID
    int file_name_length = strlen(name);
    if (file_name_length > HDIMG_ID_POS) { // HD[N]
      int tmp_id = name[HDIMG_ID_POS] - '0';

      if(tmp_id >= 0 && tmp_id < NUM_ANSIID) {
        id = tmp_id; // If valid id, set it.
      } else {
        logmsg("-- Ignoring ", name, ", invalid ANSI ID: ", tmp_id);
        continue;
      }
    }

    // Add the directory name to get the full file path
    char fullname[MAX_FILE_PATH * 2 + 2] = {0};
    strncpy(fullname, imgdir.c_str(), MAX_FILE_PATH);
    if (fullname[strlen(fullname) - 1] != '/') strcat(fullname, "/");
    strcat(fullname, name);

    // Check whether this ANSI ID has been seen before
    if (idsSeen & (1 << id)) {
      logmsg("-- Ignoring ", fullname, ", ANSI ID ", id, " is already in use!");
      continue;
    }

    g_ansi_settings.initDevice(id);

    logmsg("-- Opening ", fullname, " for id:", id);

    if (g_ansi_settings.getDevicePreset(id) != DEV_PRESET_NONE) {
        logmsg("---- Using device preset: ", g_ansi_settings.getDevicePresetName(id));
    }

    bool imageReady = ansiDiskOpenHDDImage(id, fullname, 1056/*XXX hardcoded apollo size.  should come from device settings*/);
    if (imageReady) {
      foundImage = true;
    } else {
      logmsg("---- Failed to load image");
    }
  }

  root.close();

#if notyet
  // Print ANSI drive map
  for (int i = 0; i < NUM_ANSIID; i++)
  {
    const S2S_TargetCfg* cfg = s2s_getConfigByIndex(i);
    
    if (cfg && (cfg->scsiId & S2S_CFG_TARGET_ENABLED))
    {
      int capacity_kB = ((uint64_t)cfg->scsiSectors * cfg->bytesPerSector) / 1024;

      logmsg("SCSI ID: ", (int)(cfg->scsiId & S2S_CFG_TARGET_ID_BITS),
            ", BlockSize: ", (int)cfg->bytesPerSector,
            ", Type: ", (int)cfg->deviceType,
            ", Quirks: ", (int)cfg->quirks,
            ", Size: ", capacity_kB, "kB");
    }
  }
#endif

  return foundImage;
}

static bool mountSDCard()
{
  // Prepare for mounting new SD card by closing all old files.
  // When switching between FAT and exFAT cards the pointers
  // are invalidated and accessing old files results in crash.
  invalidate_ini_cache();
  g_logfile.close();
  // notyet ansiDiskCloseSDCardImages();

  // Check for the common case, FAT filesystem as first partition
  if (SD.begin(SD_CONFIG))
  {
    reload_ini_cache(CONFIGFILE);
    return true;
  }

  return false;
}

static void reinitANSI()
{
  g_log_debug = inifile.getbool("ANSI", "Debug", false);

  ansiDiskResetImages();
  {
    readConfig();
    findHDDImages();

    // Error if there are 0 image files
    if (ansiDiskCheckAnyImagesConfigured())
    {
      // Ok, there is an image, turn LED on for the time it takes to perform init
      LED_ON();
      delay(100);
    }
    else
    {
      logmsg("No valid image files found!");
      blinkStatus(BLINK_ERROR_NO_IMAGES);
    }
  }

  // notyet
  // scsiPhyReset();
  // scsiDiskInit();
  // scsiInit();
}

// Place all the setup code that requires the SD card to be initialized here
// Which is pretty much everything after platform_init and and platform_late_init
static void tansi_setup_sd_card()
{    
  g_sdcard_present = mountSDCard();

  if(!g_sdcard_present)
  {
    logmsg("SD card init failed, sdErrorCode: ", (int)SD.sdfs.sdErrorCode(),
           " sdErrorData: ", (int)SD.sdfs.sdErrorData());

    do
    {
      blinkStatus(BLINK_ERROR_NO_SD_CARD);
      delay(1000);
      // notyet platform_reset_watchdog();
      g_sdcard_present = mountSDCard();
    } while (!g_sdcard_present);
    logmsg("SD card init succeeded after retry");
  }

  if (g_sdcard_present)
  {
    if (SD.sdfs.clusterCount() == 0)
    {
      logmsg("SD card without filesystem!");
    }

    print_sd_info();
    
    char presetName[32];
    ini_gets("ANSI", "System", "", presetName, sizeof(presetName), CONFIGFILE);
    ansi_system_settings_t *cfg = g_ansi_settings.initSystem(presetName);
#if notyet
    int boot_delay_ms = cfg->initPreDelay;
    if (boot_delay_ms > 0)
    {
      logmsg("Pre ANSI init boot delay in millis: ", boot_delay_ms);
      delay(boot_delay_ms);
    }
#endif
    platform_post_sd_card_init();
    reinitANSI();
    

#if notyet    
    boot_delay_ms = cfg->initPostDelay;
    if (boot_delay_ms > 0)
    {
      logmsg("Post ANSI init boot delay in millis: ", boot_delay_ms);
      delay(boot_delay_ms);
    }
#endif
  }

  if (g_sdcard_present)
  {
    init_logfile();
    if (ini_getbool("ANSI", "DisableStatusLED", false, CONFIGFILE))
    {
      platform_disable_led();
    }
  }

  // Counterpart for the LED_ON in reinitANSI().
  LED_OFF();
}

extern "C" void tansi_setup(void)
{
  delay(1000);
  logmsg("TANSI " TANSI_FW_VERSION " " __DATE__ " " __TIME__);
  logmsg("SD card: ", g_sdcard_present ? "present" : "not found");
  logmsg("Debug: ", g_log_debug ? "enabled" : "disabled");

  platform_init();
  platform_late_init();
  tansi_setup_sd_card();
  platform_post_sd_card_init();

  logmsg("Setup complete.");
}

extern "C" void tansi_main_loop(void)
{
  platform_poll();
  ansi_poll();
}

