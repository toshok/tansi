/**
 * Portions - Copyright (C) 2023 Eric Helgeson
 * ZuluSCSI™ - Copyright (c) 2022-2023 Rabbit Hole Computing™
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

/* Access layer to image files associated with a SCSI device.
 * Currently supported image storage modes:
 *
 * - Files on SD card
 * - Raw SD card partitions
 * - Microcontroller flash ROM drive
 */

#pragma once
#include <SD.h>
#include <SdFat.h>
#include <stdint.h>
#include <unistd.h>

// SD card sector size is always 512 bytes
#define SD_SECTOR_SIZE 512

// This class wraps SdFat library FsFile to allow access
// through either FAT filesystem or as a raw sector range.
//
// Raw access is activated by using filename like "RAW:0:12345"
// where the numbers are the first and last sector.
//
// If the platform supports a ROM drive, it is activated by using
// filename "ROM:".
class ImageBackingStore {
  public:
    // Empty image, cannot be accessed
    ImageBackingStore();

    // Parse image file parameters from filename.
    // Special filename formats:
    //    RAW:start:end
    //    ROM:
    ImageBackingStore(const char* filename, uint32_t scsi_block_size);

    // Can the image be read?
    bool isOpen();

    // Can the image be written?
    bool isWritable();

#if notyet
    // Is this internal ROM drive in microcontroller flash?
    bool isRom();

    // Is this backed by raw passthrough
    bool isRaw();
#endif

    // Close the image so that .isOpen() will return false.
    bool close();

    // Return image size in bytes
    uint64_t size();

    // Check if the image sector range is contiguous, and the image is on
    // SD card, return the sector numbers.
    bool contiguousRange(uint32_t* bgnSector, uint32_t* endSector);

    // Set current position for following read/write operations
    bool seek(uint64_t pos);

    // Read data from the image file, returns number of bytes read, or negative
    // on error.
    ssize_t read(void* buf, size_t count);

    // Write data to image file, returns number of bytes written, or negative on
    // error.
    ssize_t write(const void* buf, size_t count);

    // Flush any pending changes to filesystem
    void flush();

    // Gets current position for following read/write operations
    // Result is only valid for regular files, not raw or flash access
    uint64_t position();

  protected:
#if notyet
    bool m_israw;
    bool m_isrom;
    bool m_isreadonly_attr;
    romdrive_hdr_t m_romhdr;
#endif
    FsFile m_fsfile;
    SdCard* m_blockdev;
    uint32_t m_bgnsector;
    uint32_t m_endsector;
    uint32_t m_cursector;
};
