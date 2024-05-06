/** 
 * SCSI2SD V6 - Copyright (C) 2013 Michael McMaster <michael@codesrc.com>
 * Copyright (C) 2014 Doug Brown <doug@downtowndougbrown.com
 * ZuluSCSI™ - Copyright (c) 2022 Rabbit Hole Computing™
 * Copyright (c) 2023 joshua stein <jcs@jcs.org>
 * 
 * It is derived from disk.h in SCSI2SD V6.
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

// SCSI disk access routines
// Implements both SCSI2SD V6 disk.h functions and some extra.

#pragma once

#include <cstdint>
#include "ImageBackingStore.h"
#include "TANSI_config.h"

// Extended configuration stored alongside the normal SCSI2SD target information
struct image_config_t//: public S2S_TargetCfg
{
    image_config_t() {};

    ImageBackingStore file;

    int ansi_id;
    // Maximum amount of bytes to prefetch
    int prefetchbytes;

    // Warning about geometry settings

    bool geometrywarningprinted;

    // Clear any image state to zeros
    void clear();

private:
    // There should be only one global instance of this struct per device, so make copy constructor private.
    image_config_t(const image_config_t&) = default;
};

// Reset all image configuration to empty reset state, close all images.
void ansiDiskResetImages();

// Close any files opened from SD card (prepare for remounting SD)
void ansiDiskCloseSDCardImages();

bool ansiDiskOpenHDDImage(int ansi_id, const char *filename, int blocksize);
void ansiDiskLoadConfig(int ansi_id);

// Checks if a filename extension is appropriate for further processing as a disk image.
// The current implementation does not check the the filename prefix for validity.
bool ansiDiskFilenameValid(const char* name);

// Returns true if there is at least one image active
bool ansiDiskCheckAnyImagesConfigured();

// Get pointer to extended image configuration based on target idx
image_config_t &ansiDiskGetImageConfig(int ansi_id);

// Start data transfer from disk image to SCSI bus
// Can be called by device type specific command implementations (such as READ CD)
void ansiDiskStartRead(uint32_t lba, uint32_t blocks);

// Start data transfer from SCSI bus to disk image
void ansiDiskStartWrite(uint32_t lba, uint32_t blocks);
