
#pragma once

#include <cstdint>
// this really needs to be configurable but for now we're only trying to support
// apollos.
#define HARD_DISK_SECTOR_SIZE 1056

struct AnsiDiskType {
    const char *name;
    uint16_t model_id;
    uint16_t cylinders;
    uint8_t heads;
    uint16_t sectors;
    uint16_t rpm;
};

extern const AnsiDiskType g_disk_types[];
extern const int g_disk_type_count;