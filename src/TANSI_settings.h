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
#pragma once

#include <cstdint>

typedef enum {
    SYS_PRESET_NONE,

    SYS_PRESET_APOLLO_DN300, // unused, but claiming space

    SYS_PRESET_COUNT,
} ansi_system_preset_t;

typedef enum {
    DEV_PRESET_NONE,

    DEV_PRESET_PRIAM_3450,
    DEV_PRESET_PRIAM_7050,

    DEV_PRESET_COUNT,
} ansi_device_preset_t;

// This struct should only have new settings added to the end
// as it maybe saved and restored directly from flash memory
struct __attribute__((__packed__)) ansi_system_settings_t {
    // Settings for host compatibility
    uint8_t quirks;
};

// This struct should only have new setting added to the end
// as it maybe saved and restored directly from flash memory
struct __attribute__((__packed__)) ansi_device_settings_t {
    // Settings that can be set on all or specific device
    int prefetchBytes;
    uint16_t bytesPerSector;
    uint16_t sectorsPerTrack;
    uint16_t headsPerCylinder;

    // ...

    uint32_t sectorSDBegin;
    uint32_t sectorSDEnd;
};

class TANSISettings {
  public:
    // Initialize settings for all devices with a preset configuration,
    //  or return the default config if unknown system type.
    // Then overwrite any settings with those in the CONFIGFILE
    ansi_system_settings_t* initSystem(const char* presetName);

    // Copy any shared device setting done the initSystemSettings as default
    // settings, or return the default config if unknown device type. Then
    // overwrite any settings with those in the CONFIGFILE
    ansi_device_settings_t* initDevice(uint8_t ansiID);
    // return the system settings struct to read values
    ansi_system_settings_t* getSystem();

    // return the device settings struct to read values
    ansi_device_settings_t* getDevice(uint8_t ansiId);

    // return the system preset enum
    ansi_system_preset_t getSystemPreset();

    // return the system preset name
    const char* getSystemPresetName();

    // return the device preset enum
    ansi_device_preset_t getDevicePreset(uint8_t ansiId);

    // return the device preset name
    const char* getDevicePresetName(uint8_t ansiId);

  protected:
    // Set default drive vendor / product info after the image file
    // is loaded and the device type is known.
    void setDefaultDriveInfo(uint8_t ansiId, const char* presetName);

    // Informative name of the preset configuration, or NULL for defaults
    ansi_system_preset_t m_sysPreset;

    // Presets for corresponding ANSI Ids e.g. [ANSI0] in the CONFIGFILE.
    ansi_device_preset_t m_devPreset[NUM_ANSIID];

    // These are setting for host compatibility
    ansi_system_settings_t m_sys;

    ansi_device_settings_t m_dev[NUM_ANSIID];
};

extern TANSISettings g_ansi_settings;

void readConfig();
