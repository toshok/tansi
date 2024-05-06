/**
 * ZuluSCSI™ - Copyright (c) 2022-2023 Rabbit Hole Computing™
 * Portions - Copyright (C) 2023 Eric Helgeson
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

#include "ImageBackingStore.h"
#include <TANSI_platform.h>
#include "TANSI_log.h"
#include "TANSI_config.h"
#include "TANSI_settings.h"
#include <minIni.h>
#include <strings.h>
#include <string.h>
#include <assert.h>

ImageBackingStore::ImageBackingStore()
{
#if notyet    
    m_israw = false;
    m_isrom = false;
    m_isreadonly_attr = false;
#endif
    m_blockdev = nullptr;
    m_bgnsector = m_endsector = m_cursector = 0;
}

ImageBackingStore::ImageBackingStore(const char *filename, uint32_t scsi_block_size): ImageBackingStore()
{
#if notyet
    if (strncasecmp(filename, "RAW:", 4) == 0)
    {
        char *endptr, *endptr2;
        m_bgnsector = strtoul(filename + 4, &endptr, 0);
        m_endsector = strtoul(endptr + 1, &endptr2, 0);

        if (*endptr != ':' || *endptr2 != '\0')
        {
            logmsg("Invalid format for raw filename: ", filename);
            return;
        }

        if ((scsi_block_size % SD_SECTOR_SIZE) != 0)
        {
            logmsg("SCSI block size ", (int)scsi_block_size, " is not supported for RAW partitions (must be divisible by 512 bytes)");
            return;
        }

        m_israw = true;
        m_blockdev = SD.sdfs.card();

        uint32_t sectorCount = SD.card()->sectorCount();
        if (m_endsector >= sectorCount)
        {
            logmsg("---- Limiting RAW image mapping to SD card sector count: ", (int)sectorCount);
            m_endsector = sectorCount - 1;
        }
    }
    else if (strncasecmp(filename, "ROM:", 4) == 0)
    {
        if (!romDriveCheckPresent(&m_romhdr))
        {
            m_romhdr.imagesize = 0;
        }
        else
        {
            m_isrom = true;
        }
    }
    else
#endif
    {
#if notyet
        m_isreadonly_attr = !!(FAT_ATTRIB_READ_ONLY & SD.attrib(filename));
        if (m_isreadonly_attr)
        {
            m_fsfile = SD.sdfs.open(filename, O_RDONLY);
            logmsg("---- Image file is read-only, writes disabled");
        }
        else
#endif
        {
            m_fsfile = SD.sdfs.open(filename, O_RDWR);
        }

        uint32_t sectorcount = m_fsfile.size() / SD_SECTOR_SIZE;
        uint32_t begin = 0, end = 0;
        if (m_fsfile.contiguousRange(&begin, &end) && end >= begin + sectorcount
            && (scsi_block_size % SD_SECTOR_SIZE) == 0)
        {
            // Convert to raw mapping, this avoids some unnecessary
            // access overhead in SdFat library.
            // If non-aligned offsets are later requested, it automatically falls
            // back to SdFat access mode.
#if notyet
            m_israw = true;
#endif
            m_blockdev = SD.sdfs.card();
            m_bgnsector = begin;

#if notyet
            if (end != begin + sectorcount)
            {
                uint32_t allocsize = end - begin + 1;
                // Due to issue #80 in ZuluSCSI version 1.0.8 and 1.0.9 the allocated size was mistakenly reported to SCSI controller.
                // If the drive was formatted using those versions, you may have problems accessing it with newer firmware.
                // The old behavior can be restored with setting  [SCSI] UseFATAllocSize = 1 in config file.

                if (g_ansi_settings.getSystem()->useFATAllocSize)
                {
                    sectorcount = allocsize;
                }
            }
#endif

            m_endsector = begin + sectorcount - 1;
            m_fsfile.flush(); // Note: m_fsfile is also kept open as a fallback.
        }
    }
}

bool ImageBackingStore::isOpen()
{
#if notyet
    if (m_israw)
        return (m_blockdev != NULL);
    else if (m_isrom)
        return (m_romhdr.imagesize > 0);
    else
#endif
        return m_fsfile.isOpen();
}

bool ImageBackingStore::isWritable()
{
#if notyet
    return !m_isrom && !m_isreadonly_attr;
#else
    return true;
#endif
}

#if notyet
bool ImageBackingStore::isRom()
{
    return m_isrom;
}


bool ImageBackingStore::isRaw()
{
    return m_israw;
}
#endif

bool ImageBackingStore::close()
{
#if notyet
    if (m_israw)
    {
        m_blockdev = nullptr;
        return true;
    }
    else if (m_isrom)
    {
        m_romhdr.imagesize = 0;
        return true;
    }
    else
    {
#endif
        return m_fsfile.close();
#if notyet
    }
#endif
}

uint64_t ImageBackingStore::size()
{
#if notyet
    if (m_israw && m_blockdev)
    {
        return (uint64_t)(m_endsector - m_bgnsector + 1) * SD_SECTOR_SIZE;
    }
    else if (m_isrom)
    {
        return m_romhdr.imagesize;
    }
    else
    {
#endif
        return m_fsfile.size();
#if notyet
    }
#endif
}

bool ImageBackingStore::contiguousRange(uint32_t* bgnSector, uint32_t* endSector)
{
#if notyet
    if (m_israw && m_blockdev)
    {
        *bgnSector = m_bgnsector;
        *endSector = m_endsector;
        return true;
    }
    else if (m_isrom)
    {
        *bgnSector = 0;
        *endSector = 0;
        return true;
    }
    else
    {
#endif
        return m_fsfile.contiguousRange(bgnSector, endSector);
#if notyet
    }
#endif
}

bool ImageBackingStore::seek(uint64_t pos)
{
#if notyet
    uint32_t sectornum = pos / SD_SECTOR_SIZE;

    if (m_israw && (uint64_t)sectornum * SD_SECTOR_SIZE != pos)
    {
        dbgmsg("---- Unaligned access to image, falling back to SdFat access mode");
        m_israw = false;
    }

    if (m_israw)
    {
        m_cursector = m_bgnsector + sectornum;
        return (m_cursector <= m_endsector);
    }
    else if (m_isrom)
    {
        uint32_t sectornum = pos / SD_SECTOR_SIZE;
        assert((uint64_t)sectornum * SD_SECTOR_SIZE == pos);
        m_cursector = sectornum;
        return m_cursector * SD_SECTOR_SIZE < m_romhdr.imagesize;
    }
    else
    {
#endif
        return m_fsfile.seek(pos);
#if notyet
    }
#endif
}

ssize_t ImageBackingStore::read(void* buf, size_t count)
{
#if notyet
    uint32_t sectorcount = count / SD_SECTOR_SIZE;
    if (m_israw && (uint64_t)sectorcount * SD_SECTOR_SIZE != count)
    {
        dbgmsg("---- Unaligned access to image, falling back to SdFat access mode");
        m_israw = false;
    }

    if (m_israw && m_blockdev)
    {
        if (m_blockdev->readSectors(m_cursector, (uint8_t*)buf, sectorcount))
        {
            m_cursector += sectorcount;
            return count;
        }
        else
        {
            return -1;
        }
    }
    else if (m_isrom)
    {
        uint32_t sectorcount = count / SD_SECTOR_SIZE;
        assert((uint64_t)sectorcount * SD_SECTOR_SIZE == count);
        uint32_t start = m_cursector * SD_SECTOR_SIZE;
        if (romDriveRead((uint8_t*)buf, start, count))
        {
            m_cursector += sectorcount;
            return count;
        }
        else
        {
            return -1;
        }
    }
    else
    {
#endif
        return m_fsfile.read(buf, count);
#if notyet
    }
#endif
}

ssize_t ImageBackingStore::write(const void* buf, size_t count)
{
#if notyet
    uint32_t sectorcount = count / SD_SECTOR_SIZE;
    if (m_israw && (uint64_t)sectorcount * SD_SECTOR_SIZE != count)
    {
        dbgmsg("---- Unaligned access to image, falling back to SdFat access mode");
        m_israw = false;
    }

    if (m_israw && m_blockdev)
    {
        if (m_blockdev->writeSectors(m_cursector, (const uint8_t*)buf, sectorcount))
        {
            m_cursector += sectorcount;
            return count;
        }
        else
        {
            return 0;
        }
    }
    else if (m_isrom)
    {
        logmsg("ERROR: attempted to write to ROM drive");
        return 0;
    }
    else  if (m_isreadonly_attr)
    {
        logmsg("ERROR: attempted to write to a read only image");
        return 0;
    }
    else
    {
#endif
        return m_fsfile.write(buf, count);
#if notyet
    }
#endif
}

void ImageBackingStore::flush()
{
#if notyet
    if (!m_israw && !m_isrom && !m_isreadonly_attr)
    {
#endif
        m_fsfile.flush();
#if notyet
    }
#endif
}

uint64_t ImageBackingStore::position()
{
#if notyet
    if (!m_israw && !m_isrom)
    {
#endif
        return m_fsfile.curPosition();
#if notyet
    }
    else
    {
        return 0;
    }
#endif
}
