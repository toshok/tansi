// Compile-time configuration parameters.
// Other settings can be set by ini file at runtime.

#pragma once

#include <TeensyANSI_platform.h>

// Use variables for version number
#define FW_VER_NUM      "0.0.1"
#define FW_VER_SUFFIX   "dev"
#define TANSI_FW_VERSION FW_VER_NUM "-" FW_VER_SUFFIX

// Configuration and log file paths
#define CONFIGFILE  "tansi.ini"
#define LOGFILE     "tansilog.txt"
#define CRASHFILE   "tansierr.txt"

// Prefix for command file to create new image (case-insensitive)
#define CREATEFILE "create"

// Log buffer size in bytes, must be a power of 2
#ifndef LOGBUFSIZE
#define LOGBUFSIZE 16384
#endif
#define LOG_SAVE_INTERVAL_MS 1000

// Watchdog timeout
// Watchdog will first issue a bus reset and if that does not help, crashdump.
#define WATCHDOG_BUS_RESET_TIMEOUT 15000
#define WATCHDOG_CRASH_TIMEOUT 30000

// HDD image file format
#define HDIMG_ID_POS  2                 // Position to embed ID number
#define HDIMG_LUN_POS 3                 // Position to embed LUN numbers
#define HDIMG_BLK_POS 5                 // Position to embed block size numbers
#define MAX_FILE_PATH 64                // Maximum file name length

// Image definition options
#define IMAGE_INDEX_MAX 9               // Maximum number of 'IMG0' style statements parsed

// SCSI config
#define NUM_SCSIID  8          // Maximum number of supported SCSI-IDs (The minimum is 0)
#define NUM_SCSILUN 1          // Maximum number of LUNs supported     (Currently has to be 1)
#define READ_PARITY_CHECK 0    // Perform read parity check (unverified)

// Default delay for SCSI phases.
// Can be adjusted in ini file
#define DEFAULT_SCSI_DELAY_US 10
#define DEFAULT_REQ_TYPE_SETUP_NS 500

// Use prefetch buffer in read requests
#ifndef PREFETCH_BUFFER_SIZE
#define PREFETCH_BUFFER_SIZE 8192
#endif

// Masks for buttons
#define EJECT_BTN_MASK (1|2)
#define USER_BTN_MASK  (4)
