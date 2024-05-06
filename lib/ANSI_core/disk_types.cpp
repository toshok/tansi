#include "disk_types.h"

static AnsiDiskType PRIAM_7050 = {
    .name = "PRIAM_7050",
    .model_id = 0x105,
    .cylinders = 1049,
    .heads = 5,
    .sectors = 12,
    .rpm = 3600,
};

static AnsiDiskType PRIAM_3450 = {
    .name = "PRIAM_3450",
    .model_id = 0x104,
    .cylinders = 525,
    .heads = 5,
    .sectors = 12,
    .rpm = 3600,
};

const AnsiDiskType g_disk_types[] = {
    PRIAM_7050,
    PRIAM_3450,
};

const int g_disk_type_count = sizeof(g_disk_types) / sizeof(g_disk_types[0]);