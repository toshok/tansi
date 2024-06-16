/**
 * ZuluSCSI™ - Copyright (c) 2023 Rabbit Hole Computing™
 * Copyright (c) 2023 Eric Helgeson
 * 
 * This file is licensed under the GPL version 3 or any later version.  
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

#include <SdFat.h>
#include <SD.h>

#include "TANSI_log.h"
#include "TANSI_config.h"
#include "TANSI_disk.h"
#include "TANSI_settings.h"
#include <strings.h>
#include <minIni.h>
#include <minIni_cache.h>
#include "disk_types.h"

minIni inifile(CONFIGFILE);

// ANSI system and device settings
TANSISettings g_ansi_settings;

const char *systemPresetName[] = {"", "DN300"};
const char *devicePresetName[] = {"", "PRIAM_7050", "PRIAM_3450"};

// Helper function for case-insensitive string compare
static bool strequals(const char *a, const char *b)
{
    return strcasecmp(a, b) == 0;
}

void TANSISettings::setDefaultDriveInfo(uint8_t ansiId, const char* presetName)
{
    char section[6] = "ANSI0";
    section[4] += ansiId;

    ansi_device_settings_t &cfgDev = m_dev[ansiId];

    bool known_preset = false;

    if (strequals(devicePresetName[DEV_PRESET_NONE], presetName)) {
        // empty preset, use default
        known_preset = true;
        m_devPreset[ansiId] = DEV_PRESET_NONE;
    } else {
        for (int i = 1; i < DEV_PRESET_COUNT; i++) {
            if (strequals(devicePresetName[i], presetName)) {

#if notyet
                driveinfo = g_disk_types[i].model_id;
#endif
                known_preset = true;
                m_devPreset[ansiId] = (ansi_device_preset_t)i;
                break;
            }
        }
    }

    if (!known_preset)
    {
        m_devPreset[ansiId] = DEV_PRESET_NONE;
        logmsg("Unknown Device preset name ", presetName, ", using default settings");
    }
}

// Read device settings
static void readIniANSIDeviceSettings(ansi_device_settings_t &cfg, const char *section)
{
    cfg.bytesPerSector = inifile.getl(section, "BytesPerSector", cfg.bytesPerSector);
    cfg.sectorsPerTrack = inifile.getl(section, "SectorsPerTrack", cfg.sectorsPerTrack);
    cfg.headsPerCylinder = inifile.getl(section, "HeadsPerCylinder", cfg.headsPerCylinder);
    cfg.prefetchBytes = inifile.getl(section, "PrefetchBytes", cfg.prefetchBytes);

    cfg.sectorSDBegin = inifile.getl(section, "SectorSDBegin", cfg.sectorSDBegin);
    cfg.sectorSDEnd = inifile.getl(section, "SectorSDEnd", cfg.sectorSDEnd);
}

ansi_system_settings_t *TANSISettings::initSystem(const char *presetName)
{
    ansi_system_settings_t &cfgSys = m_sys;

    // System-specific defaults
    if (strequals(systemPresetName[SYS_PRESET_NONE], presetName)) {
        // Preset name is empty, use default configuration
        m_sysPreset = SYS_PRESET_NONE;
    } else if (strequals(systemPresetName[SYS_PRESET_APOLLO_DN300], presetName)) {
        m_sysPreset = SYS_PRESET_APOLLO_DN300;
        // cfgSys.quirks = 
    } else {
        m_sysPreset = SYS_PRESET_NONE;
        logmsg("Unknown System preset name ", presetName, ", using default settings");
    }

    // Read settings from ini file that apply to all ANSI device
    cfgSys.quirks = inifile.getl("ANSI", "Quirks", cfgSys.quirks);

    return &cfgSys;
}

ansi_device_settings_t* TANSISettings::initDevice(uint8_t ansiId) {
    ansi_device_settings_t& cfg = m_dev[ansiId];
    char section[6] = "ANSI0";
    section[4] = '0' + ansiId;

    std::string presetName = inifile.gets(section, "Device", "");

    setDefaultDriveInfo(ansiId, presetName.c_str());
    readIniANSIDeviceSettings(cfg, section);

#if notyet
    if (cfg.serial[0] == '\0')
    {
        // Use SD card serial number
        cid_t sd_cid;
        uint32_t sd_sn = 0;
        if (SD.card()->readCID(&sd_cid))
        {
            sd_sn = sd_cid.psn();
        }

        memset(cfg.serial, 0, sizeof(cfg.serial));
        const char *nibble = "0123456789ABCDEF";
        cfg.serial[0] = nibble[(sd_sn >> 28) & 0xF];
        cfg.serial[1] = nibble[(sd_sn >> 24) & 0xF];
        cfg.serial[2] = nibble[(sd_sn >> 20) & 0xF];
        cfg.serial[3] = nibble[(sd_sn >> 16) & 0xF];
        cfg.serial[4] = nibble[(sd_sn >> 12) & 0xF];
        cfg.serial[5] = nibble[(sd_sn >>  8) & 0xF];
        cfg.serial[6] = nibble[(sd_sn >>  4) & 0xF];
        cfg.serial[7] = nibble[(sd_sn >>  0) & 0xF];
    }
#endif

    return &cfg;
}

ansi_system_settings_t *TANSISettings::getSystem() {
    return &m_sys;
}

ansi_device_settings_t *TANSISettings::getDevice(uint8_t ansiId) {
    return &m_dev[ansiId];
}

ansi_system_preset_t TANSISettings::getSystemPreset() {
    return m_sysPreset;
}

const char* TANSISettings::getSystemPresetName() {
    return systemPresetName[m_sysPreset];
}

ansi_device_preset_t TANSISettings::getDevicePreset(uint8_t ansiId) {
    return m_devPreset[ansiId];
}

const char* TANSISettings::getDevicePresetName(uint8_t ansiId) {
    return devicePresetName[m_devPreset[ansiId]];
}


void readConfig() {
    if (SD.exists(CONFIGFILE)) {
        logmsg("Reading configuration from " CONFIGFILE);
        // first read ansi settings

        // then read platform settings

        // then read per-disk settings
        for (int i = 0; i < NUM_ANSIID; i++) {
            logmsg("Loading config for disk ", i);
            ansiDiskLoadConfig(i);
        }
    } else {
        logmsg("Config file " CONFIGFILE " not found, using defaults");
        // fill in the defaults for ansi settings

        // fill in the defaults for platform settings

        // disk settings?
    }
}
