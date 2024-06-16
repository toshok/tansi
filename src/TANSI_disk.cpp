/**
 * SCSI2SD V6 - Copyright (C) 2013 Michael McMaster <michael@codesrc.com>
 * Portions Copyright (C) 2014 Doug Brown <doug@downtowndougbrown.com>
 * Portions Copyright (C) 2023 Eric Helgeson
 * ZuluSCSI™ - Copyright (c) 2022-2023 Rabbit Hole Computing™
 *
 * This file is licensed under the GPL version 3 or any later version. 
 * It is derived from disk.c in SCSI2SD V6
 *
 * https://www.gnu.org/licenses/gpl-3.0.html
 * ----
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version. 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details. 
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 **/

// This file implements the main ANSI disk emulation and data streaming.
// It is derived from disk.c in ZuluSCSI, which is derived from SCSI2SD V6.

#include "TANSI_disk.h"
#include "ImageBackingStore.h"
#include "TANSI_config.h"
#include "TANSI_log.h"
#include "TANSI_settings.h"
// #include "QuirksCheck.h"
#include <SdFat.h>
#include <assert.h>
#include <minIni.h>
#include <string.h>
#include <strings.h>

#ifndef PLATFORM_MAX_SCSI_SPEED
#define PLATFORM_MAX_SCSI_SPEED S2S_CFG_SPEED_ASYNC_50
#endif

// This can be overridden in platform file to set the size of the transfers
// used when reading from SCSI bus and writing to SD card.
// When SD card access is fast, these are usually better increased.
// If SD card access is roughly same speed as SCSI bus, these can be left at 512
#ifndef PLATFORM_OPTIMAL_MIN_SD_WRITE_SIZE
#define PLATFORM_OPTIMAL_MIN_SD_WRITE_SIZE 512
#endif

#ifndef PLATFORM_OPTIMAL_MAX_SD_WRITE_SIZE
#define PLATFORM_OPTIMAL_MAX_SD_WRITE_SIZE 1024
#endif

// Optimal size for the last write in a write request.
// This is often better a bit smaller than PLATFORM_OPTIMAL_SD_WRITE_SIZE
// to reduce the dead time between end of SCSI transfer and finishing of SD
// write.
#ifndef PLATFORM_OPTIMAL_LAST_SD_WRITE_SIZE
#define PLATFORM_OPTIMAL_LAST_SD_WRITE_SIZE 512
#endif

// Optimal size for read block from SCSI bus
// For platforms with nonblocking transfer, this can be large.
// For Akai MPC60 compatibility this has to be at least 5120
#ifndef PLATFORM_OPTIMAL_SCSI_READ_BLOCK_SIZE
#ifdef PLATFORM_SCSIPHY_HAS_NONBLOCKING_READ
#define PLATFORM_OPTIMAL_SCSI_READ_BLOCK_SIZE 65536
#else
#define PLATFORM_OPTIMAL_SCSI_READ_BLOCK_SIZE 8192
#endif
#endif

#if notyet
#ifndef PLATFORM_SCSIPHY_HAS_NONBLOCKING_READ
// For platforms that do not have non-blocking read from SCSI bus
void scsiStartRead(uint8_t* data, uint32_t count, int* parityError) {
    scsiRead(data, count, parityError);
}
void scsiFinishRead(uint8_t* data, uint32_t count, int* parityError) {}
bool scsiIsReadFinished(const uint8_t* data) { return true; }
#endif
#endif

/***********************/
/* Image configuration */
/***********************/

static image_config_t g_DiskImages[NUM_ANSIID];

void ansiDiskResetImages() {
#if notyet
    for (int i = 0; i < S2S_MAX_TARGETS; i++) {
        g_DiskImages[i].clear();
    }
#endif
}

void image_config_t::clear() {
    static const image_config_t empty; // Statically zero-initialized
    *this = empty;
}

void scsiDiskCloseSDCardImages() {
#if notyet
    for (int i = 0; i < S2S_MAX_TARGETS; i++) {
        if (!g_DiskImages[i].file.isRom()) {
            g_DiskImages[i].file.close();
        }

        g_DiskImages[i].cuesheetfile.close();
    }
#endif
}

// remove path and extension from filename
void extractFileName(const char* path, char* output) {

    const char *lastSlash, *lastDot;
    int fileNameLength;

    lastSlash = strrchr(path, '/');
    if (!lastSlash)
        lastSlash = path;
    else
        lastSlash++;

    lastDot = strrchr(lastSlash, '.');
    if (lastDot && (lastDot > lastSlash)) {
        fileNameLength = lastDot - lastSlash;
        strncpy(output, lastSlash, fileNameLength);
        output[fileNameLength] = '\0';
    } else {
        strcpy(output, lastSlash);
    }
}

void setNameFromImage(image_config_t& img, const char* filename) {
#if notyet
    char image_name[MAX_FILE_PATH];

    extractFileName(filename, image_name);
    memset(img.vendor, 0, 8);
    strncpy(img.vendor, image_name, 8);
    memset(img.prodId, 0, 8);
    strncpy(img.prodId, image_name + 8, 8);
#endif
}

// Load values for target image configuration from given section if they exist.
// Otherwise keep current settings.
static void ansiDiskSetImageConfig(uint8_t target_idx) {
#if notyet
    image_config_t& img = g_DiskImages[target_idx];
    scsi_system_settings_t* devSys = g_scsi_settings.getSystem();
    scsi_device_settings_t* devCfg = g_scsi_settings.getDevice(target_idx);
    img.scsiId = target_idx;
    memset(img.vendor, 0, sizeof(img.vendor));
    memset(img.prodId, 0, sizeof(img.prodId));
    memset(img.revision, 0, sizeof(img.revision));
    memset(img.serial, 0, sizeof(img.serial));

    img.deviceType = devCfg->deviceType;
    img.deviceTypeModifier = devCfg->deviceTypeModifier;
    img.sectorsPerTrack = devCfg->sectorsPerTrack;
    img.headsPerCylinder = devCfg->headsPerCylinder;
    img.quirks = devSys->quirks;
    img.rightAlignStrings = devCfg->rightAlignStrings;
    img.name_from_image = devCfg->nameFromImage;
    img.prefetchbytes = devCfg->prefetchBytes;
    img.reinsert_on_inquiry = devCfg->reinsertOnInquiry;
    img.reinsert_after_eject = devCfg->reinsertAfterEject;
    img.ejectButton = devCfg->ejectButton;
    img.vendorExtensions = devCfg->vendorExtensions;

#ifdef ENABLE_AUDIO_OUTPUT
    uint16_t vol = devCfg->vol;
    // Set volume on both channels
    audio_set_volume(target_idx, (vol << 8) | vol);
#endif

    memcpy(img.vendor, devCfg->vendor, sizeof(img.vendor));
    memcpy(img.prodId, devCfg->prodId, sizeof(img.prodId));
    memcpy(img.revision, devCfg->revision, sizeof(img.revision));
    memcpy(img.serial, devCfg->serial, sizeof(img.serial));
#endif
}

bool ansiDiskOpenHDDImage(int ansi_id, const char* filename, int blocksize) {
    image_config_t& img = g_DiskImages[ansi_id];
    ansiDiskSetImageConfig(ansi_id);
    img.file = ImageBackingStore(filename, blocksize);

    if (img.file.isOpen()) {
#if notyet
        img.bytesPerSector = blocksize;
        img.scsiSectors = img.file.size() / blocksize;
#endif
        img.ansi_id = ansi_id;
#if notyet
        img.sdSectorStart = 0;
#endif

#if notyet
        if (img.ansiSectors == 0) {
            logmsg("---- Error: image file ", filename, " is empty");
            img.file.close();
            return false;
        }
#endif
        uint32_t sector_begin = 0, sector_end = 0;
        if (img.file.contiguousRange(&sector_begin, &sector_end)) {
            dbgmsg("---- Image file is contiguous, SD card sectors ",
                   (int)sector_begin, " to ", (int)sector_end);
        } else {
            logmsg("---- WARNING: file ", filename,
                   " is not contiguous. This will increase read latency.");
        }

        logmsg("---- Configuring as disk drive");

#if notyet
        quirksCheck(&img);
#endif

        if (img.prefetchbytes > 0) {
            logmsg("---- Read prefetch enabled: ", (int)img.prefetchbytes,
                   " bytes");
        } else {
            logmsg("---- Read prefetch disabled");
        }

        return true;
    } else {
        logmsg("---- Failed to load image '", filename, "', ignoring");
        return false;
    }
}

#if notyet
static void checkDiskGeometryDivisible(image_config_t& img) {
    if (!img.geometrywarningprinted) {
        uint32_t sectorsPerHeadTrack =
            img.sectorsPerTrack * img.headsPerCylinder;
        if (img.scsiSectors % sectorsPerHeadTrack != 0) {
            logmsg(
                "WARNING: Host used command ", scsiDev.cdb[0],
                " which is affected by drive geometry. Current settings are ",
                (int)img.sectorsPerTrack, " sectors x ",
                (int)img.headsPerCylinder,
                " heads = ", (int)sectorsPerHeadTrack, " but image size of ",
                (int)img.scsiSectors,
                " sectors is not divisible. This can cause error messages in "
                "diagnostics tools.");
            img.geometrywarningprinted = true;
        }
    }
}
#endif

bool ansiDiskFilenameValid(const char* name) {
    // Check file extension.  only `.img` is permissible.
    const char* extension = strrchr(name, '.');
    if (!extension) {
        return false;
    }

    if (strcasecmp(extension, ".img")) {
        // invalid extension
        return false;
    }

    return true;
}

// Load values for target configuration from given section if they exist.
// Otherwise keep current settings.
static void ansiDiskSetConfig(int ansi_id) {
#if notyet
    image_config_t& img = g_DiskImages[ansi_id];
    img.ansi_id = ansi_id;
#endif

    ansiDiskSetImageConfig(ansi_id);

    g_ansi_settings.initDevice(ansi_id);
}

#if notyet
// Finds filename with the lowest lexical order _after_ the given filename in
// the given folder. If there is no file after the given one, or if there is
// no current file, this will return the lowest filename encountered.
static int findNextImageAfter(image_config_t& img, const char* dirname,
                              const char* filename, char* buf, size_t buflen) {
    FsFile dir;
    if (dirname[0] == '\0') {
        logmsg("Image directory name invalid for ID",
               (img.scsiId & S2S_CFG_TARGET_ID_BITS));
        return 0;
    }
    if (!dir.open(dirname)) {
        logmsg("Image directory '", dirname, "' couldn't be opened");
        return 0;
    }
    if (!dir.isDir()) {
        logmsg("Can't find images in '", dirname, "', not a directory");
        dir.close();
        return 0;
    }
    if (dir.isHidden()) {
        logmsg("Image directory '", dirname, "' is hidden, skipping");
        dir.close();
        return 0;
    }

    char first_name[MAX_FILE_PATH] = {'\0'};
    char candidate_name[MAX_FILE_PATH] = {'\0'};
    FsFile file;
    while (file.openNext(&dir, O_RDONLY)) {
        if (file.isDir())
            continue;
        if (!file.getName(buf, MAX_FILE_PATH)) {
            logmsg("Image directory '", dirname, "' had invalid file");
            continue;
        }
        if (!scsiDiskFilenameValid(buf))
            continue;
        if (file.isHidden()) {
            logmsg("Image '", dirname, "/", buf, "' is hidden, skipping file");
            continue;
        }

        // keep track of the first item to allow wrapping
        // without having to iterate again
        if (first_name[0] == '\0' || strcasecmp(buf, first_name) < 0) {
            strncpy(first_name, buf, sizeof(first_name));
        }

        // discard if no selected name, or if candidate is before (or is)
        // selected
        if (filename[0] == '\0' || strcasecmp(buf, filename) <= 0)
            continue;

        // if we got this far and the candidate is either 1) not set, or 2) is a
        // lower item than what has been encountered thus far, it is the best
        // choice
        if (candidate_name[0] == '\0' || strcasecmp(buf, candidate_name) < 0) {
            strncpy(candidate_name, buf, sizeof(candidate_name));
        }
    }

    if (candidate_name[0] != '\0') {
        img.image_index++;
        strncpy(img.current_image, candidate_name, sizeof(img.current_image));
        strncpy(buf, candidate_name, buflen);
        return strlen(candidate_name);
    } else if (first_name[0] != '\0') {
        img.image_index = 0;
        strncpy(img.current_image, first_name, sizeof(img.current_image));
        strncpy(buf, first_name, buflen);
        return strlen(first_name);
    } else {
        logmsg("Image directory '", dirname, "' was empty");
        img.image_directory = false;
        return 0;
    }
}
#endif

void ansiDiskLoadConfig(int ansi_id) { ansiDiskSetConfig(ansi_id); }

bool ansiDiskCheckAnyImagesConfigured() {
    for (int i = 0; i < NUM_ANSIID; i++) {
        if (g_DiskImages[i].file.isOpen()
#if notyet
            && (g_DiskImages[i].scsiId & S2S_CFG_TARGET_ENABLED)
#endif
        ) {
            return true;
        }
    }
    return false;
}

image_config_t& ansiDiskGetImageConfig(int target_idx) {
    assert(target_idx >= 0 && target_idx < NUM_ANSIID);
    return g_DiskImages[target_idx];
}

#if notyet
void s2s_configInit(S2S_BoardCfg* config) {
    {
        char tmp[64];

        // Get default values from system preset, if any
        ini_gets("ANSI", "System", "", tmp, sizeof(tmp), CONFIGFILE);
        scsi_system_settings_t* sysCfg = g_scsi_settings.initSystem(tmp);

        if (g_scsi_settings.getSystemPreset() != SYS_PRESET_NONE) {
            logmsg("Active configuration (using system preset \"",
                   g_scsi_settings.getSystemPresetName(), "\"):");
        } else {
            logmsg("Active configuration:");
        }

        memset(config, 0, sizeof(S2S_BoardCfg));
        memcpy(config->magic, "BCFG", 4);
        config->flags = 0;
        config->startupDelay = 0;
        config->selectionDelay = sysCfg->selectionDelay;
        config->flags6 = 0;
        config->scsiSpeed = PLATFORM_MAX_SCSI_SPEED;

        int maxSyncSpeed = sysCfg->maxSyncSpeed;
        if (maxSyncSpeed < 5 && config->scsiSpeed > S2S_CFG_SPEED_ASYNC_50)
            config->scsiSpeed = S2S_CFG_SPEED_ASYNC_50;
        else if (maxSyncSpeed < 10 && config->scsiSpeed > S2S_CFG_SPEED_SYNC_5)
            config->scsiSpeed = S2S_CFG_SPEED_SYNC_5;

        logmsg("-- SelectionDelay = ", (int)config->selectionDelay);

        if (sysCfg->enableUnitAttention) {
            logmsg("-- EnableUnitAttention = Yes");
            config->flags |= S2S_CFG_ENABLE_UNIT_ATTENTION;
        } else {
            logmsg("-- EnableUnitAttention = No");
        }

        if (sysCfg->enableSCSI2) {
            logmsg("-- EnableSCSI2 = Yes");
            config->flags |= S2S_CFG_ENABLE_SCSI2;
        } else {
            logmsg("-- EnableSCSI2 = No");
        }

        if (sysCfg->enableSelLatch) {
            logmsg("-- EnableSelLatch = Yes");
            config->flags |= S2S_CFG_ENABLE_SEL_LATCH;
        } else {
            logmsg("-- EnableSelLatch = No");
        }

        if (sysCfg->mapLunsToIDs) {
            logmsg("-- MapLunsToIDs = Yes");
            config->flags |= S2S_CFG_MAP_LUNS_TO_IDS;
        } else {
            logmsg("-- MapLunsToIDs = No");
        }
    }

    extern "C" void s2s_debugInit(void) {}

    extern "C" void s2s_configPoll(void) {}

    extern "C" void s2s_configSave(int scsiId, uint16_t byesPerSector) {
        // Modification of config over SCSI bus is not implemented.
    }

    extern "C" const S2S_TargetCfg* s2s_getConfigByIndex(int index) {
        if (index < 0 || index >= S2S_MAX_TARGETS) {
            return NULL;
        } else {
            return &g_DiskImages[index];
        }
    }

    extern "C" const S2S_TargetCfg* s2s_getConfigById(int scsiId) {
        int i;
        for (i = 0; i < S2S_MAX_TARGETS; ++i) {
            const S2S_TargetCfg* tgt = s2s_getConfigByIndex(i);
            if ((tgt->scsiId & S2S_CFG_TARGET_ID_BITS) == scsiId &&
                (tgt->scsiId & S2S_CFG_TARGET_ENABLED)) {
                return tgt;
            }
        }
        return NULL;
    }
#endif